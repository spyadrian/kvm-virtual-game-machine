#ifndef _GFX_H_
#define _GFX_H_

#include <stdint.h>

// --- DEFINE ---

// ...

// --- FUNCTION ---

/**
 * @brief Initializes the virtual graphics system with specified width and height using paravirtualization.
 *
 * Sets up the virtual graphics display with the desired dimensions through a paravirtualized hypercall,
 * allowing for efficient graphics setup in the guest environment.
 *
 * @param width  Width of the display in pixels.
 * @param height Height of the display in pixels.
 */
void gfx_pv_init(uint32_t width, uint32_t height);

/**
 * @brief Initializes the virtual graphics system with specified width and height in emulated mode.
 *
 * Configures the graphics display with the provided dimensions through an emulated hypercall,
 * simulating a hardware-based initialization process.
 *
 * @param width  Width of the display in pixels.
 * @param height Height of the display in pixels.
 */
void gfx_phys_init(uint32_t width, uint32_t height);

#endif // !_TIMER_H_
