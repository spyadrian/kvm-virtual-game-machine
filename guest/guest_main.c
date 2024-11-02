#include <stdint.h>

#include "idt.h"
#include "x86.h"

#include "gfx/gfx.h"
#include "timer/timer.h"
#include "console/console.h"

// If PV == 1 => uses paravirtualized drivers (when available)
// If PV == 0 => uses physical (hardware) drivers (when available)
#if PV == 1
#define timer_wait    timer_pv_wait
#define console_send  console_pv_send
#define gfx_init      gfx_pv_init
#else
#define timer_wait    timer_phys_wait
#define console_send  console_pv_send
#define gfx_init      gfx_phys_init
#endif

void guest_main() {

    idt_init();  // Initialize interrupt subsystem
    sti();       // Enable hardware interrupts

    // - console -
    char *str = "Hi from the guest !";
    console_send(str);

    // - timer -
    timer_wait(1e6);

    // - gfx init -
    gfx_pv_init(1920, 1080);

    timer_wait(1e7);
}

