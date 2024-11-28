#ifndef _SPRITE_H_
#define _SPRITE_H_

#include <stdint.h>

// --- DEFINE ---

// ...

// --- FUNCTION ---

void sprite_pv_init(uint8_t id, uint8_t *data, uint32_t width, uint32_t height);

void sprite_pv_visibility(uint8_t id, uint8_t toggle);

void sprite_pv_position(uint8_t id, uint32_t x, uint32_t y);

void sprite_phys_init(uint8_t id, uint8_t *data, uint32_t width, uint32_t height);

void sprite_phys_visibility(uint8_t id, uint8_t toggle);

void sprite_phys_position(uint8_t id, uint32_t x, uint32_t y);

#endif // !_SPRITE_H_
