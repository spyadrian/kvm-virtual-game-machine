#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdint.h>

// --- DEFINE ---

// ...

// --- FUNCTION ---

/**
 * @brief Waits for a specified duration using a paravirtualized timer.
 *
 * Pauses execution within the guest for a set time in microseconds, 
 * leveraging a paravirtualized hypercall for efficient timing.
 *
 * @param us Duration of the wait in microseconds.
 */
void timer_pv_wait(uint32_t us);

/**
 * @brief Waits for a specified duration using an emulated timer.
 *
 * Delays execution within the guest for the specified time in microseconds, 
 * simulating a hardware-based timing wait.
 *
 * @param us Duration of the wait in microseconds.
 */
void timer_phys_wait(uint32_t us);

#endif // !_TIMER_H_
