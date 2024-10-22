#define _OPERATION_H_
#ifdef _OPERATION_H_

#include <stdint.h>
#include <unistd.h>


// --- DEFINE ---

#define STATE_MACHINE_NUM 2

typedef enum {

    OP_WRITE_EQUAL,
    OP_WRITE_STORE,
    OP_READ_INJECT,
    OP_EMUL_END,
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

void op_callback_console_conclude(void *addr);

void op_callback_timer_store(void *addr);

void op_callback_timer_conclude(void *addr);

void op_callback_gfx_init_store_w(void *addr);

void op_callback_gfx_init_store_h(void *addr);

void op_callback_gfx_init_conclude(void *addr);

#endif // _OPERATION_H_
