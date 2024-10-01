#include <stdint.h>
#include "idt.h"
#include "utils.h"
#include "x86.h"

// If PV == 1 => uses paravirtualized drivers (when available)
// If PV == 0 => uses physical (hardware) drivers (when available)
#if PV == 1
#define timer_sleep timer_sleep_pv
#else
#define timer_sleep timer_sleep_phys
#endif

// Dummy examples
void timer_sleep_pv(int usec) {}
void timer_sleep_phys(int usec) {}

void guest_main() {
    idt_init();  // Initialize interrupt subsystem
    sti();       // Enable hardware interrupts

    timer_sleep(100000);
}
