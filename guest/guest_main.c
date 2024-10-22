#include <stdint.h>

#include "idt.h"
#include "utils.h"
#include "x86.h"
#include "pmio.h"
#include "shared/hypercall_params.h"

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

    // paravirtualized console

    char *str = "abcdefgh";
    hyper_virtual_console_params_t param_console;
    param_console.msg = (uint64_t)str;
    memcpy((void *)HYPERCALL_SHARED_ADDR, (void *)&param_console, sizeof(param_console));

    outb(HYPERCALL_PMIO_ADDR, HYPERCALL_CODE_CONSOLE);

    // paravirtualized timer

    hyper_timer_sleep_params_t param_timer;
    param_timer.us = 1e6;
    memcpy((void *)HYPERCALL_SHARED_ADDR, (void *)&param_timer, sizeof(param_timer));

    outb(HYPERCALL_PMIO_ADDR, HYPERCALL_CODE_TIMER);

    // emulated timer

    outw(REG_TIMER_CMD, 20);
    outd(REG_TIMER_DATA, 1e6);

    // paravirtualized gfx init 

    hyper_init_gfx_params_t param_gfx;
    param_gfx.width = 1920;
    param_gfx.height = 1080;
    memcpy((void *)HYPERCALL_SHARED_ADDR, (void *)&param_gfx, sizeof(param_gfx));

    outb(HYPERCALL_PMIO_ADDR, HYPERCALL_CODE_GFX_INIT);

    // emulated gfx init

    // TODO:

    // timer_sleep(100000);
}
