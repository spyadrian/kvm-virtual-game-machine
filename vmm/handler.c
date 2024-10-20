#include "handler.h"

#include <err.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "shared/hypercall_params.h"

// --- DEFINE ---

state_handler_t const STATE_HANDLERS[] = {

    [KVM_EXIT_HLT] = handle_halt,
    [KVM_EXIT_IO] = handle_pmio,
    [KVM_EXIT_MMIO] = handle_mmio,
};

static state_t const STATE_TIMER[] = {

    { OP_WRITE_EQUAL, 0x43, 20, 2, NULL },
    { OP_WRITE_EQUAL, 0x40, 0,  4, NULL },
    { OP_WRITE_EQUAL, 0,    0,  0, NULL },
};

static state_t const STATE_VGA[] = {

    { OP_WRITE_EQUAL, 0x3C2, 0x67,  1, NULL },
    { OP_WRITE_EQUAL, 0x3C4, 0xF02, 2, NULL },
    { OP_WRITE_EQUAL, 0x3CE, 0x506, 2, NULL },
    { OP_WRITE_EQUAL, 0x3DA, 0x80,  1, NULL },
    { OP_WRITE_EQUAL, 0x3C0, 0x20,  1, NULL },
    { OP_WRITE_EQUAL, 0x3C5, 0x0F,  1, NULL },
    { OP_WRITE_EQUAL, 0,     0,     0, NULL },
};

// --- STATIC FUNCTION ---

static uint32_t retrieve_value(uint8_t *addr, uint8_t size) {

    uint32_t value;

    switch (size) {

        case 1:
            value = *(uint8_t *)addr;
        break;

        case 2:
            value = *(uint16_t *)addr;
        break;

        case 4:
            value = *(uint32_t *)addr;
        break;

        default:
            errx(1, "Unsupported size trying to retrieve value.\n");
        break;
    }

    return value;
}

// --- FUNCTION ---

// - VMM EXIT REASON HANDLER -

void handle_halt(struct kvm_run *run, uint8_t *shared_buf, uint8_t *mem) {

    // printf("KVM_EXIT_HLT\n");
}

void handle_pmio(struct kvm_run *run, uint8_t *shared_buf, uint8_t *mem) {

    if (run->io.direction == KVM_EXIT_IO_OUT) {

        uint32_t value = retrieve_value((uint8_t *)run + run->io.data_offset, run->io.size);
        printf("PMIO guest write: \tsize=%d  \tport=0x%x \tvalue=%d\n", run->io.size * 8, run->io.port, value);

        switch (run->io.port) {

            case HYPERCALL_PMIO_ADDR:
                handle_hypercall(value, shared_buf, mem);
            break;

            case 0x43:
            break;

            case 0x40:
            break;
        }
    }
}

void handle_mmio(struct kvm_run *run, uint8_t *shared_buf, uint8_t *mem) {

    if (run->mmio.is_write) {

        uint32_t value = retrieve_value((uint8_t *)run->mmio.data, run->mmio.len);
        printf("MMIO guest write: \taddr=0x%llx \tlen=%d \tvalue=%d\n", run->mmio.phys_addr, run->mmio.len * 8, value);
    }
}

// - HYPERCALL HANDLER -

void handle_hypercall(uint32_t code, uint8_t *shared_buf, uint8_t *mem) {

    switch (code) {

        case HYPERCALL_CODE_CONSOLE:
            handle_hypercall_console(shared_buf, mem);
        break;

        case HYPERCALL_CODE_TIMER:
            handle_hypercall_timer(shared_buf);
        break;

        case HYPERCALL_CODE_GFX_INIT: 
            handle_hypercall_gfx_init(shared_buf);
        break;

        default:
            // TODO:
        break;
    }
}

void handle_hypercall_console(uint8_t *shared_buf, uint8_t *mem) {

    hyper_virtual_console_params_t *p_consol = (hyper_virtual_console_params_t *)shared_buf;

    printf("hypercall : sending a message...\n");
    printf("%s\n", p_consol->msg + mem);
}

void handle_hypercall_timer(uint8_t *shared_buf) {

    hyper_timer_sleep_params_t *p_timer = (hyper_timer_sleep_params_t *)shared_buf;

    printf("hypercall : setting up a %dus timer...\n", p_timer->us);
    sleep(p_timer->us / 1e6);
}

void handle_hypercall_gfx_init(uint8_t *shared_buf) {

    hyper_init_gfx_params_t *p_init_gfx = (hyper_init_gfx_params_t *)shared_buf;

    printf("hypercall : initializing gfx...\n");
    // TODO:
}
