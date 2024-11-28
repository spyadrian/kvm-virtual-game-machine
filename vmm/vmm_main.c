#include <SDL2/SDL_render.h>
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
#include <pthread.h>
#include <semaphore.h>

#include "gfx.h"
#include "handler.h"
#include "operation.h"
#include "shared/hypercall_params.h"

// -- DEFINE --

#define KVM_API_VERSION   12
#define RAM_SIZE          (512 * 1024)

#define SEM_THREAD  0

sem_t sem_gfx;
uint32_t width_gfx;
uint32_t height_gfx;
gfx_context_t *ctxt;
sprite_t sprites[MAX_SPRITES];

char *disk_path;

static uint8_t *mem, *shared_buf;
static struct kvm_run *run;
static int vcpufd;

// -- THREAD --

static void *thread_hypercall() {

    bool done = false;
    while (!done) {

        if (ioctl(vcpufd, KVM_RUN, NULL) == -1) err(1, "KVM_RUN");

        state_handler_t handler = STATE_HANDLERS[run->exit_reason];

        if (handler) {

            handler(run, shared_buf, mem, &done);
        }
        else {

            errx(1, "exit_reason = 0x%x", run->exit_reason);
        }
    }

    return NULL;
}

// -- MAIN --

int main(int argc, char* argv[])
{
    int kvmfd, vmfd;
    struct kvm_sregs sregs;
    int32_t mmap_size;

    // -- 0. Handle Args --
    if (argc != 3) err(1, "Number of args invalide");
    char *file_path = argv[1];
    disk_path = argv[2];

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
    if (fstat(binfd, &st) < 0) err(1, "Error getting file size");

    // Check if the file size exceeds the available memory
    if (st.st_size > RAM_SIZE) err(1, "Error: GuestOS binary size exceeds 512KB\n");

    // Read the file into the provided memory (mem)
    ssize_t bytes_read = read(binfd, mem, st.st_size);
    if (bytes_read < 0) err(1, "Error reading guestOS binary file");

    // Check if all bytes were read
    if (bytes_read != st.st_size) err(1, "Error: Only %ld of %ld bytes read from file\n", bytes_read, st.st_size);

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

    sem_init(&sem_gfx, SEM_THREAD, 0);

    pthread_t t_hypercall;
    pthread_create(&t_hypercall, NULL, thread_hypercall, NULL);

    sem_wait(&sem_gfx);

    // -- 8. Manage gfx --

    memset(sprites, 0, sizeof(sprites));

    ctxt = gfx_create("Basic Example", width_gfx, height_gfx);
    if (!ctxt) err(1, "KVM_SET_REGS");

    while (1) {

        // gfx_background_update(ctxt);

        for (uint8_t i = 0; i < MAX_SPRITES; i++) {

            if (sprites[i].toggle == 1) {

                if (sprites[i].tex == NULL)
                    sprites[i].tex = gfx_sprite_create(ctxt, sprites[i].data, sprites[i].width, sprites[i].height);

                gfx_sprite_render(ctxt, sprites[i].tex, sprites[i].x, sprites[i].y, sprites[i].width, sprites[i].height);
            }
        }

        gfx_present(ctxt);
    }

    // -- X. Finalize --
    pthread_join(t_hypercall, NULL);

    gfx_destroy(ctxt);
    sem_destroy(&sem_gfx);

    return 0;
}
