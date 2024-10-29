#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdint.h>

// --- DEFINE ---

// ...

// --- FUNCTION ---

void timer_pv_wait(uint32_t us);

void timer_phys_wait(uint32_t us);

#endif // !_TIMER_H_
