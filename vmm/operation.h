#ifndef _OPERATION_H_
#define _OPERATION_H_

#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>

// --- DEFINE ---

#define STATE_MACHINE_NUM 3

typedef enum {

    OP_WRITE_EQUAL,       // Perform a write operation and check if the written value matches the expected value
    OP_WRITE_STORE,       // Store the written value into a specific location (used in callbacks)
    OP_WRITE_STORE_LOOP,  // Store the written value into a specific location multiple time
    OP_READ_INJECT,       // Inject a value into the guest from a specific address
    OP_EMUL_END,          // End of the state machine, with an callback for completion
} operation_t;

typedef struct {

    operation_t op;                 // the operation to perform
    uint64_t addr;                  // the address written to/read from
    uint32_t value;                 // the expected written value or value to inject
    uint8_t size;                   // the size of the operation (8, 16 or 32 bits)
    void (*callback)(void *addr);   // custom function that would be executed at the end of the state
} state_t;

extern bool flag_stop_loop;
extern char *disk_path;
extern state_t *STATE_ALL_STARTERS[];

// --- FUNCITON ---

/**
 * @brief Callback for concluding a console operation.
 *
 * Processes the final steps of a console operation, such as outputting messages.
 *
 * @param addr Pointer to the operation's data.
 */
void op_callback_console_conclude(void *addr);

/**
 * @brief Callback for storing timer data.
 *
 * Saves the timer configuration value provided by the operation.
 *
 * @param addr Pointer to the data containing the timer configuration.
 */
void op_callback_timer_store(void *addr);

/**
 * @brief Callback for concluding a timer operation.
 *
 * Finalizes the timer setup by triggering the timer with the specified delay.
 *
 * @param addr Pointer to the operation's data.
 */
void op_callback_timer_conclude(void *addr);

/**
 * @brief Callback for storing the width during graphics initialization.
 *
 * Saves the width value provided by the operation for graphics setup.
 *
 * @param addr Pointer to the width data.
 */
void op_callback_gfx_init_store_w(void *addr);

/**
 * @brief Callback for storing the height during graphics initialization.
 *
 * Saves the height value provided by the operation for graphics setup.
 *
 * @param addr Pointer to the height data.
 */
void op_callback_gfx_init_store_h(void *addr);

/**
 * @brief Callback for concluding graphics initialization.
 *
 * Finalizes graphics setup with the stored width and height values.
 *
 * @param addr Pointer to the operation's data.
 */
void op_callback_gfx_init_conclude(void *addr);

/**
 * @brief Callback for preparing an IDE operation.
 *
 * Initializes the IDE state and prepares the loop buffer.
 *
 * @param addr Pointer to the operation's data.
 */
void op_callback_ide_prepare(void *addr);

/**
 * @brief Callback for storing the first sector value during IDE operation.
 *
 * Saves part of the sector index for an IDE write operation.
 *
 * @param addr Pointer to the sector data.
 */
void op_callback_ide_store_sector_1(void *addr);

/**
 * @brief Callback for storing the second sector value during IDE operation.
 *
 * Saves additional bits of the sector index for an IDE write operation.
 *
 * @param addr Pointer to the sector data.
 */
void op_callback_ide_store_sector_2(void *addr);

/**
 * @brief Callback for storing the third sector value during IDE operation.
 *
 * Saves further bits of the sector index for an IDE write operation.
 *
 * @param addr Pointer to the sector data.
 */
void op_callback_ide_store_sector_3(void *addr);

/**
 * @brief Callback for storing the fourth sector value during IDE operation.
 *
 * Completes the sector index for an IDE write operation.
 *
 * @param addr Pointer to the sector data.
 */
void op_callback_ide_store_sector_4(void *addr);

/**
 * @brief Callback for storing data in a loop during IDE operation.
 *
 * Writes data into the loop buffer during an IDE write operation.
 *
 * @param addr Pointer to the data being stored.
 */
void op_callback_ide_store_data(void *addr);

/**
 * @brief Callback for concluding an IDE operation.
 *
 * Finalizes an IDE operation by writing data to the specified sector.
 *
 * @param addr Pointer to the operation's data.
 */
void op_callback_ide_conclude(void *addr);

/**
 * @brief Callback for preparing sprite initialization.
 *
 * Prepares the sprite data buffer and resets the loop state.
 *
 * @param addr Pointer to the operation's data.
 */
void op_callback_sprite_init_prepare(void *addr);

/**
 * @brief Callback for storing the sprite ID during initialization.
 *
 * Saves the sprite's ID for the initialization process.
 *
 * @param addr Pointer to the sprite ID data.
 */
void op_callback_sprite_init_store_id(void *addr);

/**
 * @brief Callback for storing the sprite width during initialization.
 *
 * Saves the sprite's width value for the initialization process.
 *
 * @param addr Pointer to the width data.
 */
void op_callback_sprite_init_store_width(void *addr);

/**
 * @brief Callback for storing the sprite height during initialization.
 *
 * Saves the sprite's height value for the initialization process.
 *
 * @param addr Pointer to the height data.
 */
void op_callback_sprite_init_store_height(void *addr);

/**
 * @brief Callback for storing sprite pixel data during initialization.
 *
 * Writes pixel data into the loop buffer during the initialization process.
 *
 * @param addr Pointer to the pixel data.
 */
void op_callback_sprite_init_store_pixels(void *addr);

/**
 * @brief Callback for concluding sprite initialization.
 *
 * Finalizes sprite initialization and applies the stored configuration.
 *
 * @param addr Pointer to the operation's data.
 */
void op_callback_sprite_init_conclude(void *addr);

/**
 * @brief Callback for storing the sprite ID during visibility update.
 *
 * Saves the sprite's ID for visibility changes.
 *
 * @param addr Pointer to the sprite ID data.
 */
void op_callback_sprite_visibility_store_id(void *addr);

/**
 * @brief Callback for storing the visibility toggle during sprite update.
 *
 * Saves the toggle value to update the sprite's visibility.
 *
 * @param addr Pointer to the toggle data.
 */
void op_callback_sprite_visibility_store_toggle(void *addr);

/**
 * @brief Callback for concluding sprite visibility update.
 *
 * Finalizes the visibility update for the sprite.
 *
 * @param addr Pointer to the operation's data.
 */
void op_callback_sprite_visibility_conclude(void *addr);

/**
 * @brief Callback for storing the sprite ID during position update.
 *
 * Saves the sprite's ID for position changes.
 *
 * @param addr Pointer to the sprite ID data.
 */
void op_callback_sprite_position_store_id(void *addr);

/**
 * @brief Callback for storing the X-coordinate during sprite position update.
 *
 * Saves the X-coordinate value for the sprite's new position.
 *
 * @param addr Pointer to the X-coordinate data.
 */
void op_callback_sprite_position_store_x(void *addr);

/**
 * @brief Callback for storing the Y-coordinate during sprite position update.
 *
 * Saves the Y-coordinate value for the sprite's new position.
 *
 * @param addr Pointer to the Y-coordinate data.
 */
void op_callback_sprite_position_store_y(void *addr);

/**
 * @brief Callback for concluding sprite position update.
 *
 * Finalizes the position update for the sprite.
 *
 * @param addr Pointer to the operation's data.
 */
void op_callback_sprite_position_conclude(void *addr);

#endif // _OPERATION_H_
