#include "sprite.h"

#include <stdint.h>
#include "shared/hypercall_params.h"

// --- DEFINE ---

// ...

// --- FUNCTION ---

void sprite_phys_init(uint8_t id, uint8_t *data, uint32_t width, uint32_t height) {

    while(*(uint32_t *)REG_SPRITE_ST != 64);

    *(uint32_t *)REG_SPRITE_CMD = 9;

    *(uint32_t *)REG_SPRITE_CMD = 27;
    *(uint32_t *)REG_SPRITE_DATA = id;

    *(uint32_t *)REG_SPRITE_DATA = width;
    *(uint32_t *)REG_SPRITE_DATA = height;

    for (uint32_t i = 0; i < width * height; i++) {

        *(uint8_t *)REG_SPRITE_DATA = *data++; // r
        *(uint8_t *)REG_SPRITE_DATA = *data++; // g
        *(uint8_t *)REG_SPRITE_DATA = *data++; // b
        *(uint8_t *)REG_SPRITE_DATA = *data++; // a
    }
}

void sprite_phys_visibility(uint8_t id, uint8_t toggle) {

    *(uint32_t *)REG_SPRITE_CMD = 14;

    *(uint32_t *)REG_SPRITE_CMD = 27;
    *(uint32_t *)REG_SPRITE_DATA = id;

    *(uint8_t *)REG_SPRITE_DATA = toggle;
}

void sprite_phys_position(uint8_t id, uint32_t x, uint32_t y) {

    *(uint32_t *)REG_SPRITE_CMD = 35;

    *(uint32_t *)REG_SPRITE_CMD = 27;
    *(uint32_t *)REG_SPRITE_DATA = id;

    *(uint32_t *)REG_SPRITE_DATA = x;
    *(uint32_t *)REG_SPRITE_DATA = y;
}
