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

// -- DEFINE --

#define KVM_API_VERSION   12
#define RAM_SIZE          (512 * 1024)

// -- MAIN --

int main(int argc, char* argv[])
{
    int kvmfd, vmfd, vcpufd;
    struct kvm_sregs sregs;
    struct kvm_run *run;
    size_t mmap_size;
    uint8_t *mem;

    // -- 0. Handle Args --
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

    // -- 3. Allocate RAM for the VM --
    mem = mmap(NULL, RAM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); // memory to hold the code
    if (!mem) err(1, "allocating guest memory");

    // -- 4. Map allocated RAM into the VM’s address space --
    struct kvm_userspace_memory_region region = {
        .slot = 0,
        .guest_phys_addr = 0,
        .memory_size = RAM_SIZE,
        .userspace_addr = (uint64_t)mem,
        .flags = 0,
    };

    int memreg_err = ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &region);
    if (memreg_err == -1) err(1, "KVM_SET_USER_MEMORY_REGION");

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
    if (mmap_size < sizeof(*run)) errx(1, "KVM_GET_VCPU_MMAP_SIZE unexpectedly small");

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

            case KVM_EXIT_HLT:
                puts("KVM_EXIT_HLT");
                done = true;
                break;

            case KVM_EXIT_IO:
                if (run->io.direction == KVM_EXIT_IO_OUT && run->io.size == 1
                        && run->io.port == 0x3f8 && run->io.count == 1)
                    putchar(*(((char *)run) + run->io.data_offset));
                else
                    errx(1, "unhandled KVM_EXIT_IO");
                break;

            case KVM_EXIT_MMIO:
                // !TODO!
                done = true;
                break;

            case KVM_EXIT_FAIL_ENTRY:
                errx(1, "KVM_EXIT_FAIL_ENTRY: hardware_entry_failure_reason = 0x%llx", 
                     (unsigned long long)run->fail_entry.hardware_entry_failure_reason);
                done = true;
                break;

            case KVM_EXIT_INTERNAL_ERROR:
                errx(1, "KVM_EXIT_INTERNAL_ERROR: suberror = 0x%x", run->internal.suberror);
                done = true;
                break;

            default:
                errx(1, "exit_reason = 0x%x", run->exit_reason);
                done = true;
                break;
        }
    }
}
