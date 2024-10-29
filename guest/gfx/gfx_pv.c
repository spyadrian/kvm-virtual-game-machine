#include "gfx.h"

#include "guest/utils.h"
#include "guest/pmio.h"
#include "shared/hypercall_params.h"

// --- DEFINE ---

// ...

// --- FUNCTION ---

void gfx_pv_init(uint32_t width, uint32_t height) {

    hyper_init_gfx_params_t param_gfx;
    param_gfx.width = width;
    param_gfx.height = height;
    memcpy((void *)HYPERCALL_SHARED_ADDR, (void *)&param_gfx, sizeof(param_gfx));

    outb(HYPERCALL_PMIO_ADDR, HYPERCALL_CODE_GFX_INIT);
}

