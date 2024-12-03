#include <stdint.h>
#include <stdbool.h>

#include "irq.h"
#include "idt.h"
#include "x86.h"
#include "pmio.h"
#include "shared/ide_regs.h"
#include "shared/hypercall_params.h"

#include "gfx/gfx.h"
#include "ide/ide.h"
#include "timer/timer.h"
#include "sprite/sprite.h"
#include "console/console.h"

#include "guest/resources/resources.h"

// --- DEFINE ---

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

uint32_t x = 50;
uint32_t y = 50;

// --- FUNCTION ---

void keyboard_handler(void) {

    #if PV == 1

        hyper_keyboard_params_t *p_keyboard = (hyper_keyboard_params_t *)HYPERCALL_SHARED_ADDR;
        char key = (char)p_keyboard->key;
    #else

        char key = (char)ind(REG_KEYBOARD_DATA);
    #endif

    // char str[125]; snprintf(str, 125, "key received : %c\n", key);
    // console_send(str);

    switch (key) {

        case 'w':

            y -= 25;
        break;

        case 's':

            y += 25;
        break;

        case 'a':

            x -= 25;
        break;

        case 'd':

            x += 25;
        break;

        default:
            // nothing
        break;
    }

    sprite_position(0, x, y);
}

void guest_main() {

    idt_init();  // Initialize interrupt subsystem

    handler_t hd = {keyboard_handler, "IRQ_KEYBOARD"};
    irq_install_handler(IRQ_KEYBOARD, hd);

    sti();       // Enable hardware interrupts

    // - console -
    char *str = "Hi from the guest !";
    console_send(str);

    // - timer -
    timer_wait(1e6);

    // - gfx init -
    gfx_init(1920, 1080);

    // - disk -
    char ide_data[SECTOR_SIZE] = "here are my data !\n";
    ide_write(0, ide_data);

    // - sprite -
    sprite_init(0, sprite_tuxjedi, SPRITE_TUXJEDI_WIDTH, SPRITE_TUXJEDI_HEIGHT);
    sprite_position(0, x, y);
    sprite_visibility(0, 1);

    // - main loop -
    char *s = "-- guest enter while 1 --";
    console_send(s);

    while (1);
    halt();
}

