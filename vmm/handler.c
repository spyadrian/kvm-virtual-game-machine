#include "handler.h"

#include <err.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
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
        state_t *state = (i == 0) ? current_state : STATE_ALL_STARTERS[i - 1];

        if (state == NULL) continue;

        // - check the validity of the retrieving state -
        switch (state->op) {

            case OP_WRITE_EQUAL:

                if (state->addr == addr && state->size == size && state->value == value)
                    return state;
            break;

            case OP_WRITE_STORE:
            case OP_WRITE_STORE_LOOP:

                if (state->addr == addr && state->size == size && state->callback != NULL)
                    return state;
            break;

            case OP_READ_INJECT:

                if (state->addr == addr && state->size == size)
                    return state;
            break;

            case OP_EMUL_END:

                return state;
            break;
        }
    }

    return NULL;
}

static void state_compute(state_t *state, uint32_t value, uint8_t *addr) {

    bool go_next = true;

    // - compute actual operation -
    if (state->callback != NULL)
        state->callback(&value);

    // - compute general operation that don't need callback -
    switch (state->op) {

        case OP_READ_INJECT:

            injecte_data(addr, state->size, state->value);
            printf("guest read: size=%d port=0x%lx [value injected by VMM=0x%x]\n",
                   state->size * 8, state->addr, state->value);
        break;

        case OP_WRITE_STORE_LOOP:

            if (flag_stop_loop == false)
                go_next = false;
        break;

        case OP_EMUL_END:
        case OP_WRITE_STORE:
        case OP_WRITE_EQUAL:
            // nothing...
        break;
    }

    // - check and compute last operation -
    if (go_next) current_state = state + 1;

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

static void wrapper_op_ide(uint8_t *shared_buf, uint8_t *mem) {

    hyper_ide_params_t *p_ide = (hyper_ide_params_t *)shared_buf;
    p_ide->data = p_ide->data + (uint64_t)mem;

    op_callback_ide_conclude(p_ide);
}

// --- FUNCTION ---

// - VMM EXIT REASON HANDLER -

void handle_halt(struct kvm_run *run, ...) {

    va_list args; va_start(args, run);

    va_arg(args, uint8_t *); va_arg(args, uint8_t *);  // skip two first args
    bool *done = va_arg(args, bool *);

    printf("guest halted\n");
    *done = true;

    va_end(args);
}

// waiting for : uint8_t *shared_buf, uint8_t *mem
void handle_pmio(struct kvm_run *run, ...) {

    va_list args; va_start(args, run);

    uint8_t *shared_buf = va_arg(args, uint8_t *);
    uint8_t *mem = va_arg(args, uint8_t *);

    if (run->io.direction == KVM_EXIT_IO_OUT) {

        uint32_t value = retrieve_data((uint8_t *)run + run->io.data_offset, run->io.size);
        printf("PMIO guest write: \tport=0x%x \tsize=%d  \tvalue=%d\n", run->io.port, run->io.size * 8, value);

        if (run->io.port == HYPERCALL_PMIO_ADDR) {

            handle_hypercall(value, shared_buf, mem);

        } else {

            current_state = state_check(run->io.port, run->io.size, value); // get and check incoming state

            if (current_state != NULL)
                state_compute(current_state, value, (uint8_t *)run + run->io.data_offset);
        }
    }
    else if (run->io.direction == KVM_EXIT_IO_IN) {

        printf("PMIO guest read: \tport=0x%x \tsize=%d\n", run->io.port, run->io.size * 8);
        current_state = state_check(run->io.port, run->io.size, 0);

        if (current_state != NULL)
            state_compute(current_state, current_state->value, (uint8_t *)run + run->io.data_offset);
    }

    va_end(args);
}

void handle_mmio(struct kvm_run *run, ...) {

    if (run->mmio.is_write) {

        uint32_t value = retrieve_data((uint8_t *)run->mmio.data, run->mmio.len);
        printf("MMIO guest write: \taddr=0x%llx \tlen=%d \tvalue=%d\n", run->mmio.phys_addr, run->mmio.len * 8, value);

        current_state = state_check(run->mmio.phys_addr, run->mmio.len, value);

        if (current_state != NULL)
            state_compute(current_state, value, run->mmio.data);
    }
    else if (!run->mmio.is_write) {

        printf("MMIO guest read: \taddr=0x%llx \tlen=%d\n", run->mmio.phys_addr, run->mmio.len * 8);
        current_state = state_check(run->mmio.phys_addr, run->mmio.len, 0);

        if (current_state != NULL)
            state_compute(current_state, current_state->value, run->mmio.data);
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

        case HYPERCALL_CODE_IDE:
            wrapper_op_ide(shared_buf, mem);
        break;

        default:
            errx(1, "Unsupported hypercall code.\n");
        break;
    }
}

