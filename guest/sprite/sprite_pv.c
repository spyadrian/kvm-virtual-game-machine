#include "sprite.h"

#include "guest/utils.h"
#include "guest/pmio.h"
#include "shared/hypercall_params.h"
#include <stdint.h>

// --- DEFINE ---

// ...

// --- FUNCTION ---

void sprite_pv_init(uint8_t id, uint8_t *data, uint32_t width, uint32_t height) {

    hyper_sprite_init_params_t param_sprite_init;
    param_sprite_init.id = id;
    param_sprite_init.data = (uint64_t)((uint32_t)data);
    param_sprite_init.width = width;
    param_sprite_init.height = height;
    memcpy((void *)HYPERCALL_SHARED_ADDR, (void *)&param_sprite_init, sizeof(param_sprite_init));

    outb(HYPERCALL_PMIO_ADDR, HYPERCALL_CODE_SPRITE_INIT);
}

void sprite_pv_visibility(uint8_t id, uint8_t toggle) {

    hyper_sprite_visibility_params_t param_sprite_vis;
    param_sprite_vis.id = id;
    param_sprite_vis.toggle = toggle;
    memcpy((void *)HYPERCALL_SHARED_ADDR, (void *)&param_sprite_vis, sizeof(param_sprite_vis));

    outb(HYPERCALL_PMIO_ADDR, HYPERCALL_CODE_SPRITE_VISI);
}

void sprite_pv_position(uint8_t id, uint32_t x, uint32_t y) {

    hyper_sprite_position_params_t param_sprite_pos;
    param_sprite_pos.id = id;
    param_sprite_pos.x = x;
    param_sprite_pos.y = y;
    memcpy((void *)HYPERCALL_SHARED_ADDR, (void *)&param_sprite_pos, sizeof(param_sprite_pos));

    outb(HYPERCALL_PMIO_ADDR, HYPERCALL_CODE_SPRITE_POS);
}
