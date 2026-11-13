#include "keyboard.h"

#include <stdint.h>

#include "guest/utils.h"
#include "guest/pmio.h"
#include "shared/hypercall_params.h"

// --- DEFINE ---

// ...

// --- FUNCTION ---

uint32_t keyboard_pv_get_keycode(void) {

    hyper_keyboard_params_t param_keyboard;
    memcpy((void *)HYPERCALL_SHARED_ADDR, (void *)&param_keyboard, sizeof(param_keyboard));

    outb(HYPERCALL_PMIO_ADDR, HYPERCALL_CODE_KEYBOARD);

    hyper_keyboard_params_t *p_keyboard = (hyper_keyboard_params_t *)HYPERCALL_SHARED_ADDR;
    return p_keyboard->key;
}
