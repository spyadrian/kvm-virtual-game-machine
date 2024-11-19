#include <stdint.h>

#include "idt.h"
#include "shared/ide_regs.h"
#include "x86.h"

#include "gfx/gfx.h"
#include "ide/ide.h"
#include "timer/timer.h"
#include "sprite/sprite.h"
#include "console/console.h"

// If PV == 1 => uses paravirtualized drivers (when available)
// If PV == 0 => uses physical (hardware) drivers (when available)
#if PV == 1
#define timer_wait          timer_pv_wait
#define console_send        console_pv_send
#define gfx_init            gfx_pv_init
#define ide_write           ide_pv_write_sector
#define sprite_init         sprite_pv_init
#define sprite_visibility   sprite_pv_visibility
#define sprite_position     sprite_pv_position
#else
#define timer_wait          timer_phys_wait
#define console_send        console_pv_send
#define gfx_init            gfx_phys_init
#define ide_write           ide_phys_write_sector
#define sprite_init         sprite_phys_init
#define sprite_visibility   sprite_phys_visibility
#define sprite_position     sprite_phys_position
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
    gfx_init(1920, 1080);

    // - disk -
    char data[SECTOR_SIZE] = "here are my data !\n";
    ide_write(0, data);

    // - sprite -
    sprite_init(0, 0x123, 200, 100);
    sprite_visibility(0, 1);
    sprite_position(0, 50, 50);

    halt();
}

