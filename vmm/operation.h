#ifndef _OPERATION_H_
#define _OPERATION_H_

#include <stdint.h>
#include <unistd.h>


// --- DEFINE ---

#define STATE_MACHINE_NUM 2

typedef enum {

    OP_WRITE_EQUAL,  // Perform a write operation and check if the written value matches the expected value
    OP_WRITE_STORE,  // Store the written value into a specific location (used in callbacks)
    OP_READ_INJECT,  // Inject a value into the guest from a specific address
    OP_EMUL_END,     // End of the state machine, with an callback for completion
} operation_t;

typedef struct {

    operation_t op;                 // the operation to perform
    uint32_t addr;                  // the address written to/read from
    uint32_t value;                 // the expected written value or value to inject
    uint8_t size;                   // the size of the operation (8, 16 or 32 bits)
    void (*callback)(void *addr);   // custom function that would be executed at the end of the state
} state_t;

extern state_t *STATE_ALL_STARTERS[];

extern state_t STATE_TIMER[];

extern state_t STATE_GFX_INIT[];

extern state_t STATE_VGA[];

// --- FUNCITON ---

/**
 * @brief Callback function triggered when the console operation concludes.
 *
 * @param addr Address containing the console message to be printed.
 */
void op_callback_console_conclude(void *addr);

/**
 * @brief Callback function to store the timer value during the state machine execution.
 *
 * @param addr Address of the timer value to be stored.
 */
void op_callback_timer_store(void *addr);

/**
 * @brief Callback function to conclude the timer operation and set the sleep timer.
 *
 * @param addr Address of the timer parameters, or `NULL` to use previously stored values.
 */
void op_callback_timer_conclude(void *addr);

/**
 * @brief Callback function to store the width during graphics initialization.
 *
 * @param addr Address of the width value to be stored.
 */
void op_callback_gfx_init_store_w(void *addr);

/**
 * @brief Callback function to store the height during graphics initialization.
 *
 * @param addr Address of the height value to be stored.
 */
void op_callback_gfx_init_store_h(void *addr);

/**
 * @brief Callback function to conclude the graphics initialization process.
 *
 * @param addr Address of the graphics parameters, or `NULL` to use previously stored values.
 */
void op_callback_gfx_init_conclude(void *addr);

#endif // _OPERATION_H_
