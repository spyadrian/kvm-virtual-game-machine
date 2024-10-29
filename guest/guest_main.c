#include <stdint.h>

#include "idt.h"
#include "utils.h"
#include "x86.h"
#include "pmio.h"
#include "shared/hypercall_params.h"

#include "ide/ide.h"
#include "gfx/gfx.h"
#include "timer/timer.h"
#include "console/console.h"

// If PV == 1 => uses paravirtualized drivers (when available)
// If PV == 0 => uses physical (hardware) drivers (when available)
#if PV == 1
#define timer_sleep timer_sleep_pv
#else
#define timer_sleep timer_sleep_phys
#endif

// Dummy examples
// void timer_sleep_pv(int usec) {}
// void timer_sleep_phys(int usec) {}

void guest_main() {

    idt_init();  // Initialize interrupt subsystem
    sti();       // Enable hardware interrupts

    // - pv console -
    char *str = "Hi from the guest !";
    console_send_pv(str);

    // - pv timer -
    timer_pv_wait(1e6);

    // - phys timer -
    timer_phys_wait(1e6);

    // - pv gfx init -
    gfx_pv_init(1920, 1080);

    // - phys gfx init -
    /* gfx_phys_init(1920, 1080); */

    // timer_sleep(100000);
}

