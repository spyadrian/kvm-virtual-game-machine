#ifndef _SPRITE_H_
#define _SPRITE_H_

#include <stdint.h>

// --- DEFINE ---

// ...

// --- FUNCTION ---

/**
 * @brief Initializes a sprite using paravirtualized (pv) calls.
 * 
 * This function initializes a sprite with the given ID, pixel data, width, and height 
 * using hypercalls for paravirtualized implementations.
 * 
 * @param id The ID of the sprite to initialize.
 * @param data Pointer to the sprite pixel data in RGBA format.
 * @param width The width of the sprite in pixels.
 * @param height The height of the sprite in pixels.
 */
void sprite_pv_init(uint8_t id, uint8_t *data, uint32_t width, uint32_t height);

/**
 * @brief Toggles the visibility of a sprite using paravirtualized (pv) calls.
 * 
 * Sets the visibility of a sprite identified by its ID using hypercalls.
 * 
 * @param id The ID of the sprite.
 * @param toggle A flag indicating visibility (0 for hidden, 1 for visible).
 */
void sprite_pv_visibility(uint8_t id, uint8_t toggle);

/**
 * @brief Sets the position of a sprite using paravirtualized (pv) calls.
 * 
 * Updates the x and y coordinates of a sprite using hypercalls.
 * 
 * @param id The ID of the sprite.
 * @param x The x-coordinate of the sprite.
 * @param y The y-coordinate of the sprite.
 */
void sprite_pv_position(uint8_t id, uint32_t x, uint32_t y);

/**
 * @brief Initializes a sprite using physical (phys) memory-mapped registers.
 * 
 * This function initializes a sprite with the given ID, pixel data, width, and height 
 * by writing directly to hardware registers.
 * 
 * @param id The ID of the sprite to initialize.
 * @param data Pointer to the sprite pixel data in RGBA format.
 * @param width The width of the sprite in pixels.
 * @param height The height of the sprite in pixels.
 */
void sprite_phys_init(uint8_t id, uint8_t *data, uint32_t width, uint32_t height);

/**
 * @brief Toggles the visibility of a sprite using physical (phys) memory-mapped registers.
 * 
 * Sets the visibility of a sprite identified by its ID by directly writing to hardware registers.
 * 
 * @param id The ID of the sprite.
 * @param toggle A flag indicating visibility (0 for hidden, 1 for visible).
 */
void sprite_phys_visibility(uint8_t id, uint8_t toggle);

/**
 * @brief Sets the position of a sprite using physical (phys) memory-mapped registers.
 * 
 * Updates the x and y coordinates of a sprite by directly writing to hardware registers.
 * 
 * @param id The ID of the sprite.
 * @param x The x-coordinate of the sprite.
 * @param y The y-coordinate of the sprite.
 */
void sprite_phys_position(uint8_t id, uint32_t x, uint32_t y);

#endif // !_SPRITE_H_
