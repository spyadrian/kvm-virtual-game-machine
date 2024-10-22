#define _STATES_H_
#ifdef _STATES_H_

#include <stdint.h>
#include <linux/kvm.h>

// --- DEFINE ---

typedef void (*state_handler_t)(struct kvm_run *run, uint8_t *shared_buf, uint8_t *mem);

extern state_handler_t const STATE_HANDLERS[];

// --- FUNCITON ---

void handle_halt(struct kvm_run *run, uint8_t *shared_buf, uint8_t *mem);

void handle_pmio(struct kvm_run *run, uint8_t *shared_buf, uint8_t *mem);

void handle_mmio(struct kvm_run *run, uint8_t *shared_buf, uint8_t *mem);

void handle_hypercall(uint32_t code, uint8_t *shared_buf, uint8_t *mem);

#endif // _STATES_H_
