#include "operation.h"

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <semaphore.h>

#include "shared/hypercall_params.h"

// --- DEFINE ---

state_t STATE_TIMER[] = {

    { OP_WRITE_EQUAL, REG_TIMER_CMD,  20, 2, NULL },
    { OP_WRITE_STORE, REG_TIMER_DATA, 0,  4, op_callback_timer_store },
    { OP_EMUL_END,    0,              0,  0, op_callback_timer_conclude },
};

state_t STATE_GFX_INIT[] = {

    { OP_READ_INJECT, REG_GFX_INIT_ST,   0, 4, NULL },
    { OP_WRITE_EQUAL, REG_GFX_INIT_CMD,  5, 4, NULL },
    { OP_WRITE_STORE, REG_GFX_INIT_DATA, 0, 4, op_callback_gfx_init_store_w},
    { OP_WRITE_STORE, REG_GFX_INIT_DATA, 0, 4, op_callback_gfx_init_store_h},
    { OP_EMUL_END,    0,                 0, 0, op_callback_gfx_init_conclude},
};

state_t *STATE_ALL_STARTERS[] = {

    &STATE_TIMER[0],
    &STATE_GFX_INIT[0],
};

static hyper_timer_sleep_params_t param_timer;
static hyper_init_gfx_params_t param_gfx_init;

// --- STATIC FUNCTION ---

// ...

// --- FUNCTION ---

void op_callback_console_conclude(void *addr) {

    hyper_virtual_console_params_t *p_consol = (hyper_virtual_console_params_t *)addr;

    printf("action : sending a message...\n");
    printf("%s\n", (char *)p_consol->msg);
}

void op_callback_timer_store(void *addr) {

    param_timer.us = *(uint32_t *)addr;
}

void op_callback_timer_conclude(void *addr) {

    hyper_timer_sleep_params_t *p_timer = (addr == NULL) ? &param_timer : (hyper_timer_sleep_params_t *)addr;

    printf("action : setting up a %dus timer...\n", p_timer->us);
    usleep(p_timer->us);
}

void op_callback_gfx_init_store_w(void *addr) {

    param_gfx_init.width = *(uint32_t *)addr;
}

void op_callback_gfx_init_store_h(void *addr) {

    param_gfx_init.height = *(uint32_t *)addr;
}

void op_callback_gfx_init_conclude(void *addr) {

    hyper_init_gfx_params_t *p_gfx_init = (addr == NULL) ? &param_gfx_init : (hyper_init_gfx_params_t *)addr;

    printf("action : initializing gfx %dx%d...\n", p_gfx_init->width, p_gfx_init->height);
    width_gfx = p_gfx_init->width;
    height_gfx = p_gfx_init->height;
    sem_post(&sem_gfx);
}
