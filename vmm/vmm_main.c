#include <err.h>
#include <stdbool.h>
#include <fcntl.h>
#include <linux/kvm.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "states.h"
#include "shared/hypercall_params.h"

// -- DEFINE --

#define KVM_API_VERSION   12
#define RAM_SIZE          (512 * 1024)

// -- MAIN --

int main(int argc, char* argv[])
{
    int kvmfd, vmfd, vcpufd;
    struct kvm_sregs sregs;
    struct kvm_run *run;
    int32_t mmap_size;
    uint8_t *mem, *shared_buf;

    // -- 0. Handle Args --
    if (argc != 2) err(1, "Number of args invalide");
    char *file_path = argv[1];

    // -- 1. Create a KVM device --
    kvmfd = open("/dev/kvm", O_RDWR | O_CLOEXEC); // get file descriptor
    if (kvmfd == -1) err(1, "/dev/kvm");

    // check stable version of the API
    int version = ioctl(kvmfd, KVM_GET_API_VERSION, NULL);
    if (version == -1) err(1, "KVM_GET_API_VERSION");
    if (version != KVM_API_VERSION) errx(1, "KVM_GET_API_VERSION %d, expected 12", version);

    // -- 2. Create a VM --
    vmfd = ioctl(kvmfd, KVM_CREATE_VM, (unsigned long)0);
    if (vmfd == -1) err(1, "KVM_CREATE_VM");

    // -- 3.1 Allocate RAM for the VM --
    mem = mmap(NULL, RAM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); // memory to hold the code
    if (!mem) err(1, "allocating guest memory");

    // -- 3.2 Map allocated RAM into the VM’s address space --
    struct kvm_userspace_memory_region region_mem = {
        .slot = 0,
        .guest_phys_addr = 0,
        .memory_size = RAM_SIZE,
        .userspace_addr = (uint64_t)mem,
        .flags = 0,
    };

    int memreg_err = ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &region_mem);
    if (memreg_err == -1) err(1, "KVM_SET_USER_MEMORY_REGION");

    // -- 4.1 Allocate shared buffer for the VM and guest --
    shared_buf = mmap(NULL, HYPERCALL_BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); // memory to hold the code
    if (!shared_buf) err(1, "allocating shared buffer memory");

    // -- 4.2 Map allocated shared buffer into the VM’s address space --
    struct kvm_userspace_memory_region region_shared_buf = {
        .slot = 1,
        .guest_phys_addr = HYPERCALL_SHARED_ADDR,
        .memory_size = HYPERCALL_BUFF_SIZE,
        .userspace_addr = (uint64_t)shared_buf,
        .flags = 0,
    };

    int sharedbufreg_err = ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &region_shared_buf);
    if (sharedbufreg_err == -1) err(1, "KVM_SET_USER_MEMORY_REGION");

    // -- 5. Load guest OS into VM’s RAM --
    // Open the binary file
    int binfd = open(file_path, O_RDONLY);
    if (binfd < 0) err(1, "Error opening guestOS binary file");

    // Get the file size using fstat
    struct stat st;
    if (fstat(binfd, &st) < 0) perror("Error getting file size");

    // Check if the file size exceeds the available memory
    if (st.st_size > RAM_SIZE) err(1, "Error: GuestOS binary size exceeds 512KB\n");

    // Read the file into the provided memory (mem)
    ssize_t bytes_read = read(binfd, mem, st.st_size);
    if (bytes_read < 0) err(1, "Error reading guestOS binary file");

    // Check if all bytes were read
    if (bytes_read != st.st_size) 
        err(1, "Error: Only %ld of %ld bytes read from file\n", bytes_read, st.st_size);

    // Close the file descriptor
    close(binfd);

    // -- 6.1. Create a vCPU --
    vcpufd = ioctl(vmfd, KVM_CREATE_VCPU, (unsigned long)0);
    if (vcpufd == -1) err(1, "KVM_CREATE_VCPU");

    // map the shared kvm_run structure and following data
    mmap_size = ioctl(kvmfd, KVM_GET_VCPU_MMAP_SIZE, NULL);
    if (mmap_size == -1) err(1, "KVM_GET_VCPU_MMAP_SIZE");
    if (mmap_size < (int32_t)sizeof(*run)) errx(1, "KVM_GET_VCPU_MMAP_SIZE unexpectedly small");

    // get memory-mapped file
    run = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, vcpufd, 0);
    if (!run) err(1, "mmap vcpu");

    // -- 6.2. Initialize vCPU registers --
    int sregs_err = ioctl(vcpufd, KVM_GET_SREGS, &sregs);
    if (sregs_err == -1) err(1, "KVM_GET_SREGS");

    // initialize segment registers to zero, via a read-modify-write of sregs
    sregs.cs.base = 0; sregs.cs.selector = 0;
    sregs.ds.base = 0; sregs.ds.selector = 0;
    sregs.es.base = 0; sregs.es.selector = 0;
    sregs.ss.base = 0; sregs.ss.selector = 0;

    sregs_err = ioctl(vcpufd, KVM_SET_SREGS, &sregs);
    if (sregs_err == -1) err(1, "KVM_SET_SREGS");

    // initialize pointer and flags
    struct kvm_regs regs = {
        .rip = 0,               // instruction pointer -> beginning of OS's code
        .rsp = RAM_SIZE,        // stack pointer -> top of RAM
        .rflags = 0x2,          // flags required by x86 architecture
    };

    int regs_err = ioctl(vcpufd, KVM_SET_REGS, &regs);
    if (regs_err == -1) err(1, "KVM_SET_REGS");

    // -- 7. Run the vCPU --

    bool done = false;
    while (!done) {
        if (ioctl(vcpufd, KVM_RUN, NULL) == -1) err(1, "KVM_RUN");

        // handle VM exits (blocking call)
        switch (run->exit_reason) {

            case KVM_EXIT_HLT:    // "halt" CPU instruction

                puts("KVM_EXIT_HLT");
                break;

            case KVM_EXIT_IO:     // Encountered a PMIO VMexit

                if (run->io.direction == KVM_EXIT_IO_OUT) {

                    uint8_t *addr = (uint8_t *)run + run->io.data_offset;
                    uint8_t code = *(uint8_t *)addr;

                    switch (code) {

                        case 1: // console

                            hyper_virtual_console_params_t *p_consol = (hyper_virtual_console_params_t *)shared_buf;
                            printf("%s\n", p_consol->msg + mem);
                            break;

                        case 2: // timer

                            hyper_timer_sleep_params_t *p_timer = (hyper_timer_sleep_params_t *)shared_buf;
                            printf("waiting %dus\n", p_timer->us);
                            sleep(p_timer->us / 1e6);
                            done = true;
                            break;
                    }
                }

                break;

            case KVM_EXIT_MMIO:   // Encountered a MMIO VMexit

                // !TODO!
                break;

            // handle errors
            case KVM_EXIT_SHUTDOWN :
            case KVM_EXIT_FAIL_ENTRY :
            case KVM_EXIT_INTERNAL_ERROR :
            default :
                errx(1, "exit_reason = 0x%x", run->exit_reason);
                done = true;
                break;
        }
    }
}
