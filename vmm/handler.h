#define _STATES_H_
#ifdef _STATES_H_

#include <stdint.h>
#include <linux/kvm.h>

// --- DEFINE ---

typedef enum {

    OP_WRITE_EQUAL,
    OP_READ_INJECT,
    OP_EMUL_END,
} operation_t;

typedef void (*state_handler_t)(struct kvm_run *run, uint8_t *shared_buf, uint8_t *mem);

typedef struct {

    operation_t op;       // the operation to perform
    uint32_t addr;        // the address written to/read from
    uint32_t value;       // the expected written value or value to inject
    uint8_t size;         // the size of the operation (8, 16 or 32 bits)
    void (*callback)();   // custom function that would be executed at the end of the state
} state_t;

extern state_handler_t const STATE_HANDLERS[];

// --- FUNCITON ---

void handle_halt(struct kvm_run *run, uint8_t *shared_buf, uint8_t *mem);

void handle_pmio(struct kvm_run *run, uint8_t *shared_buf, uint8_t *mem);

void handle_mmio(struct kvm_run *run, uint8_t *shared_buf, uint8_t *mem);

void handle_hypercall(uint32_t code, uint8_t *shared_buf, uint8_t *mem);

void handle_hypercall_console(uint8_t *shared_buf, uint8_t *mem);

void handle_hypercall_timer(uint8_t *shared_buf);

void handle_hypercall_gfx_init(uint8_t *shared_buf);

#endif // !_STATES_H_
