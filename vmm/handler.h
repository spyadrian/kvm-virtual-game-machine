#ifndef _STATES_H_
#define _STATES_H_

#include <stdint.h>
#include <stdbool.h>
#include <linux/kvm.h>

// --- DEFINE ---

typedef void (*state_handler_t)(struct kvm_run *run, ...);

extern state_handler_t const STATE_HANDLERS[];
extern uint32_t last_key_pressed;

// --- FUNCITON ---

/**
 * @brief Handles the virtual machine halt (HLT) instruction exit.
 *
 * @param run        Pointer to the `kvm_run` structure, holding VM exit information.
 * @param shared_buf Buffer shared between the host and guest.
 * @param mem        Pointer to the VM's memory.
 */
void handle_halt(struct kvm_run *run, ...);

/**
 * @brief Handles the I/O port access (PMIO) exit from the virtual machine.
 *
 * This function processes writes to I/O ports, and if necessary, triggers state transitions or executes hypercalls.
 *
 * @param run        Pointer to the `kvm_run` structure, holding VM exit information.
 * @param shared_buf Buffer shared between the host and guest.
 * @param mem        Pointer to the VM's memory.
 */
void handle_pmio(struct kvm_run *run, ...);

/**
 * @brief Handles the memory-mapped I/O (MMIO) exit from the virtual machine.
 *
 * This function processes MMIO writes and triggers state transitions as needed based on the address and value.
 *
 * @param run        Pointer to the `kvm_run` structure, holding VM exit information.
 * @param shared_buf Buffer shared between the host and guest.
 * @param mem        Pointer to the VM's memory.
 */
void handle_mmio(struct kvm_run *run, ...);

/**
 * @brief Handles hypercalls issued by the virtual machine.
 *
 * This function processes hypercalls based on the provided code and invokes the appropriate handler (e.g., 
 * for console output, timer setup, graphics initialization or ...).
 *
 * @param code       Hypercall code indicating the operation to be performed.
 * @param shared_buf Buffer shared between the host and guest.
 * @param mem        Pointer to the VM's memory.
 */
void handle_hypercall(uint32_t code, uint8_t *shared_buf, uint8_t *mem);

#endif // _STATES_H_
