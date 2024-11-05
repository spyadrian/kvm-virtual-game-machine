#include "gfx.h"

#include <stdint.h>
#include "shared/hypercall_params.h"

// --- DEFINE ---

// ...

// --- FUNCTION ---

void gfx_phys_init(uint32_t width, uint32_t height) {

    while (*(uint32_t *)REG_GFX_INIT_ST == 42);

    *(uint32_t *)REG_GFX_INIT_CMD = 5;
    *(uint32_t *)REG_GFX_INIT_DATA = width;
    *(uint32_t *)REG_GFX_INIT_DATA = height;
}
