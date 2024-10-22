#include "handler.h"

#include <err.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>

#include "operation.h"
#include "shared/hypercall_params.h"

// --- DEFINE ---

state_handler_t const STATE_HANDLERS[] = {

    [KVM_EXIT_HLT] = handle_halt,
    [KVM_EXIT_IO] = handle_pmio,
    [KVM_EXIT_MMIO] = handle_mmio,
};

static state_t *current_state = NULL;

// --- STATIC FUNCTION ---

// - UTILS -

static uint32_t retrieve_data(uint8_t *addr, uint8_t size) {

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
            errx(1, "Unsupported size trying to retrieve data.\n");
        break;
    }

    return value;
}

static void injecte_data(uint8_t *addr, uint8_t size, uint32_t value) {

    switch (size) {

        case 1:
            *((uint8_t *)addr) = (uint8_t)value;
        break;

        case 2:
            *((uint16_t *)addr) = (uint16_t)value;
        break;

        case 4:
            *((uint32_t *)addr) = (uint32_t)value;
        break;

        default:
            errx(1, "Unsupported size trying to inject data.\n");
        break;
    }
}

static state_t *state_check(uint64_t addr, uint8_t size, uint32_t value) {

    for (uint32_t i = 0; i < STATE_MACHINE_NUM + 1; i++) {

        // - get next state or first state of one of all state machine available -
        if (i == 0 && current_state == NULL) continue;

        state_t *next_state = (i == 0) ? current_state : STATE_ALL_STARTERS[i - 1];

        // - check the validity of the next state -
        switch (next_state->op) {

            case OP_WRITE_EQUAL:

                if (next_state->addr == addr && next_state->size == size && next_state->value == value)
                    return next_state;
            break;

            case OP_WRITE_STORE:

                if (next_state->addr == addr && next_state->size == size && next_state->callback != NULL)
                    return next_state;
            break;

            case OP_EMUL_END:
            case OP_READ_INJECT:
                // nothing...
            break;
        }
    }

    return NULL;
}

static void state_compute(state_t *state, uint32_t value) {

    switch (state->op) {

        case OP_WRITE_STORE:

            state->callback(&value);
        break;

        case OP_READ_INJECT:

            injecte_data((uint8_t *)&state->addr, state->size, state->value);
            printf("PMIO guest read: size=%d port=0x%x [value injected by VMM=0x%x]\n", 
                   state->size * 8, state->addr, state->value);
        break;

        case OP_EMUL_END:
        case OP_WRITE_EQUAL:
            // nothing...
        break;
    }

    // - check and compute last operation -
    current_state = state + 1;
    if (current_state->op == OP_EMUL_END) {

        if (current_state->callback != NULL)
            current_state->callback(NULL);

        current_state = NULL; 
    }
}

// - OPERATION WRAPPER -

static void wrapper_op_console(uint8_t *shared_buf, uint8_t *mem) {

    hyper_virtual_console_params_t *p_consol = (hyper_virtual_console_params_t *)shared_buf;
    p_consol->msg = p_consol->msg + (uint64_t)mem;

    op_callback_console_conclude(p_consol);
}

static void wrapper_op_timer(uint8_t *shared_buf) {

    hyper_timer_sleep_params_t *p_timer = (hyper_timer_sleep_params_t *)shared_buf;
    op_callback_timer_conclude(p_timer);
}

static void wrapper_op_gfx_init(uint8_t *shared_buf) {

    hyper_init_gfx_params_t *p_init_gfx = (hyper_init_gfx_params_t *)shared_buf;
    op_callback_gfx_init_conclude(p_init_gfx);
}

// --- FUNCTION ---

// - VMM EXIT REASON HANDLER -

void handle_halt(struct kvm_run *run, uint8_t *shared_buf, uint8_t *mem) {

    // printf("KVM_EXIT_HLT\n");
}

void handle_pmio(struct kvm_run *run, uint8_t *shared_buf, uint8_t *mem) {

    if (run->io.direction == KVM_EXIT_IO_OUT) {

        uint32_t value = retrieve_data((uint8_t *)run + run->io.data_offset, run->io.size);
        printf("PMIO guest write: \tport=0x%x \tsize=%d  \tvalue=%d\n", run->io.port, run->io.size * 8, value);

        if (run->io.port == HYPERCALL_PMIO_ADDR) {

            handle_hypercall(value, shared_buf, mem);

        } else {

            current_state = state_check(run->io.port, run->io.size, value);

            if (current_state != NULL)
                state_compute(current_state, value);
        }
    }
}

void handle_mmio(struct kvm_run *run, uint8_t *shared_buf, uint8_t *mem) {

    if (run->mmio.is_write) {

        uint32_t value = retrieve_data((uint8_t *)run->mmio.data, run->mmio.len);
        printf("MMIO guest write: \taddr=0x%llx \tlen=%d \tvalue=%d\n", run->mmio.phys_addr, run->mmio.len * 8, value);

        current_state = state_check(run->mmio.phys_addr, run->mmio.len, value);

        if (current_state != NULL)
            state_compute(current_state, value);
    }
}

// - HYPERCALL HANDLER -

void handle_hypercall(uint32_t code, uint8_t *shared_buf, uint8_t *mem) {

    switch (code) {

        case HYPERCALL_CODE_CONSOLE:
            wrapper_op_console(shared_buf, mem);
        break;

        case HYPERCALL_CODE_TIMER:
            wrapper_op_timer(shared_buf);
        break;

        case HYPERCALL_CODE_GFX_INIT: 
            wrapper_op_gfx_init(shared_buf);
        break;

        default:
            errx(1, "Unsupported hypercall code.\n");
        break;
    }
}

