#include "operation.h"

#include <err.h>
#include "gfx.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#include "vmm/handler.h"
#include "shared/ide_regs.h"
#include "shared/hypercall_params.h"

// --- DEFINE ---

#define UNUSED(x) (void)(x)

#define BUF_SIZE (1920 * 1080 * 4)

state_t STATE_TIMER[] = {

    { OP_WRITE_EQUAL, REG_TIMER_CMD,  20, 2, NULL },
    { OP_WRITE_STORE, REG_TIMER_DATA, 0,  4, op_callback_timer_store },
    { OP_EMUL_END,    0,              0,  0, op_callback_timer_conclude },
};

state_t STATE_GFX_INIT[] = {

    { OP_READ_INJECT, REG_GFX_INIT_ST,   0, 4, NULL },
    { OP_WRITE_EQUAL, REG_GFX_INIT_CMD,  5, 4, NULL },
    { OP_WRITE_STORE, REG_GFX_INIT_DATA, 0, 4, op_callback_gfx_init_store_w },
    { OP_WRITE_STORE, REG_GFX_INIT_DATA, 0, 4, op_callback_gfx_init_store_h },
    { OP_EMUL_END,    0,                 0, 0, op_callback_gfx_init_conclude },
};

state_t STATE_IDE[] = {

    { OP_READ_INJECT,      REG_IDE_ST,       0x40, 1, op_callback_ide_prepare },
    { OP_WRITE_EQUAL,      REG_IDE_DATA + 2, 1,    1, NULL },
    { OP_WRITE_STORE,      REG_IDE_DATA + 3, 0,    1, op_callback_ide_store_sector_1 },
    { OP_WRITE_STORE,      REG_IDE_DATA + 4, 0,    1, op_callback_ide_store_sector_2 },
    { OP_WRITE_STORE,      REG_IDE_DATA + 5, 0,    1, op_callback_ide_store_sector_3 },
    { OP_WRITE_STORE,      REG_IDE_DATA + 6, 0,    1, op_callback_ide_store_sector_4 },
    { OP_WRITE_EQUAL,      REG_IDE_CMD,      0x30, 1, NULL },
    { OP_READ_INJECT,      REG_IDE_ST,       0x40, 1, NULL },
    { OP_WRITE_STORE_LOOP, REG_IDE_DATA,     0,    2, op_callback_ide_store_data },
    { OP_EMUL_END,         0,                0,    0, op_callback_ide_conclude },
};

state_t STATE_SPRITE_INIT[] = {

    { OP_READ_INJECT,      REG_SPRITE_ST,   64,    4, op_callback_sprite_init_prepare },
    { OP_WRITE_EQUAL,      REG_SPRITE_CMD,  0x9,  4, NULL },
    { OP_WRITE_EQUAL,      REG_SPRITE_CMD,  0x1B, 4, NULL },
    { OP_WRITE_STORE,      REG_SPRITE_DATA, 0,    4, op_callback_sprite_init_store_id },
    { OP_WRITE_STORE,      REG_SPRITE_DATA, 0,    4, op_callback_sprite_init_store_width },
    { OP_WRITE_STORE,      REG_SPRITE_DATA, 0,    4, op_callback_sprite_init_store_height },
    { OP_WRITE_STORE_LOOP, REG_SPRITE_DATA, 0,    1, op_callback_sprite_init_store_pixels },
    { OP_EMUL_END,         0,               0,    0, op_callback_sprite_init_conclude },
};

state_t STATE_SPRITE_VIS[] = {

    { OP_WRITE_EQUAL, REG_SPRITE_CMD,  0xE,  4, NULL },
    { OP_WRITE_EQUAL, REG_SPRITE_CMD,  0x1B, 4, NULL },
    { OP_WRITE_STORE, REG_SPRITE_DATA, 0,    4, op_callback_sprite_visibility_store_id },
    { OP_WRITE_STORE, REG_SPRITE_DATA, 0,    1, op_callback_sprite_visibility_store_toggle },
    { OP_EMUL_END,    0,               0,    0, op_callback_sprite_visibility_conclude },
};

state_t STATE_SPRITE_POS[] = {

    { OP_WRITE_EQUAL, REG_SPRITE_CMD,  35,   4, NULL },
    { OP_WRITE_EQUAL, REG_SPRITE_CMD,  0x1B, 4, NULL },
    { OP_WRITE_STORE, REG_SPRITE_DATA, 0,    4, op_callback_sprite_position_store_id },
    { OP_WRITE_STORE, REG_SPRITE_DATA, 0,    4, op_callback_sprite_position_store_x },
    { OP_WRITE_STORE, REG_SPRITE_DATA, 0,    4, op_callback_sprite_position_store_y },
    { OP_EMUL_END,    0,               0,    0, op_callback_sprite_position_conclude },
};

state_t STATE_KEYBOARD[] = {

    { OP_READ_INJECT_KEY, REG_KEYBOARD_DATA, 0, 4, NULL },
    { OP_EMUL_END,        0,                 0, 0, NULL },
};

state_t *STATE_ALL_STARTERS[] = {

    &STATE_TIMER[0],
    &STATE_GFX_INIT[0],
    &STATE_IDE[0],
    &STATE_SPRITE_INIT[0],
    &STATE_SPRITE_VIS[0],
    &STATE_SPRITE_POS[0],
    &STATE_KEYBOARD[0],
};

bool flag_stop_loop = false;

extern sem_t sem_gfx;
extern uint32_t width_gfx;
extern uint32_t height_gfx;
extern gfx_context_t *ctxt;
extern sprite_t sprites[MAX_SPRITES];

extern char *disk_path;

static hyper_timer_sleep_params_t param_timer;
static hyper_init_gfx_params_t param_gfx_init;
static hyper_ide_params_t param_ide;
static hyper_sprite_init_params_t param_sprite_init;
static hyper_sprite_visibility_params_t param_sprite_vis;
static hyper_sprite_position_params_t param_sprite_pos;

// store the index of the actual loop
static uint64_t loop_idx = BUF_SIZE;
static uint8_t loop_buf_8[BUF_SIZE];
static uint16_t *loop_buf_16 = (uint16_t *)loop_buf_8;
// static uint32_t *loop_buf_32 = (uint32_t *)loop_buf_8;

// --- FUNCTION ---

// === CONSOLE ===

void op_callback_console_conclude(void *addr) {

    hyper_virtual_console_params_t *p_consol = (hyper_virtual_console_params_t *)addr;

    printf("OPERATION : sending a message...\n");
    printf("%s\n", (char *)p_consol->msg);
}


// === TIMER ===

void op_callback_timer_store(void *addr) {

    param_timer.us = *(uint32_t *)addr;
}

void op_callback_timer_conclude(void *addr) {

    hyper_timer_sleep_params_t *p_timer = (addr == NULL) ? &param_timer : (hyper_timer_sleep_params_t *)addr;

    printf("OPERATION : setting up a %dus timer...\n", p_timer->us);
    usleep(p_timer->us);
}


// === GFX INIT ===

void op_callback_gfx_init_store_w(void *addr) {

    param_gfx_init.width = *(uint32_t *)addr;
}

void op_callback_gfx_init_store_h(void *addr) {

    param_gfx_init.height = *(uint32_t *)addr;
}

void op_callback_gfx_init_conclude(void *addr) {

    hyper_init_gfx_params_t *p_gfx_init = (addr == NULL) ? &param_gfx_init : (hyper_init_gfx_params_t *)addr;

    printf("OPERATION : initializing gfx %dx%d...\n", p_gfx_init->width, p_gfx_init->height);
    width_gfx = p_gfx_init->width;
    height_gfx = p_gfx_init->height;
    sem_post(&sem_gfx);

    while (ctxt == NULL); // wait to be sure that the gfx is init
}


// === IDE ===

void op_callback_ide_prepare(void *addr) {
    UNUSED(addr);

    flag_stop_loop = false;
    loop_idx = 0;

    memset(loop_buf_8, 0, sizeof(loop_buf_8));
    param_ide.data = (uint64_t)loop_buf_8;
}

void op_callback_ide_store_sector_1(void *addr) {

    param_ide.sector_idx = *(uint8_t *)addr;
}

void op_callback_ide_store_sector_2(void *addr) {

    param_ide.sector_idx |= (*(uint8_t *)addr) << 8;
}

void op_callback_ide_store_sector_3(void *addr) {

    param_ide.sector_idx |= (*(uint8_t *)addr) << 16;
}

void op_callback_ide_store_sector_4(void *addr) {

    param_ide.sector_idx |= ((*(uint8_t *)addr) & 0x0F) << 24;
    // what to do with the mode ? -> nothing
}

void op_callback_ide_store_data(void *addr) {

    loop_buf_16[loop_idx] = *(uint16_t *)addr;
    loop_idx++;

    if (loop_idx == MAX_LOOP_IDE_DATA) {

        loop_idx = 0;
        flag_stop_loop = true;
    }
}

void op_callback_ide_conclude(void *addr) {

    hyper_ide_params_t *p_ide = (addr == NULL) ? &param_ide : (hyper_ide_params_t *)addr;
    printf("OPERATION : writing data on sector %d\n", p_ide->sector_idx);

    // check sector range
    if (p_ide->sector_idx > MAX_SECTOR_NUM) err(1, "IDE sector is out of range");

    FILE *disk = fopen(disk_path, "r+b");
    if (!disk) err(1, "Failed opening the disk file");

    // calculate the offset for the sector
    uint64_t offset = (uint64_t)p_ide->sector_idx * SECTOR_SIZE;

    // Seek to the sector position
    if (fseek(disk, offset, SEEK_SET) != 0) {
        fclose(disk);
        err(1, "IDE seek failed");
    }

    // Write 512 bytes to the specified sector
    size_t bytes_written = fwrite((void *)p_ide->data, 1, SECTOR_SIZE, disk);
    if (bytes_written != SECTOR_SIZE) {
        fclose(disk);
        err(1, "IDE failed to write full sector");
    }

    fclose(disk);
}


// === SPRITE INIT ===

void op_callback_sprite_init_prepare(void *addr) {
    UNUSED(addr);

    flag_stop_loop = false;
    loop_idx = 0;

    memset(loop_buf_8, 0, sizeof(loop_buf_8));
    param_sprite_init.data = (uint64_t)loop_buf_8;
}

void op_callback_sprite_init_store_id(void *addr) {

    param_sprite_init.id = *(uint32_t *)addr;
}

void op_callback_sprite_init_store_width(void *addr) {

    param_sprite_init.width = *(uint32_t *)addr;
}

void op_callback_sprite_init_store_height(void *addr) {

    param_sprite_init.height = *(uint32_t *)addr;
}

void op_callback_sprite_init_store_pixels(void *addr) {

    loop_buf_8[loop_idx] = *(uint8_t *)addr;
    loop_idx++;

    if (loop_idx == param_sprite_init.width * param_sprite_init.height * 4) {

        loop_idx = 0;
        flag_stop_loop = true;
    }
}

void op_callback_sprite_init_conclude(void *addr) {

    hyper_sprite_init_params_t *p_sp_init = (addr == NULL) ? &param_sprite_init : (hyper_sprite_init_params_t *)addr;
    printf("OPERATION : initializing sprite %d with size %dx%d...\n", p_sp_init->id, p_sp_init->width, p_sp_init->height);

    sprites[p_sp_init->id].width = p_sp_init->width;
    sprites[p_sp_init->id].height = p_sp_init->height;
    sprites[p_sp_init->id].data = (uint8_t *)p_sp_init->data;
}


// === SPRITE VISIBILITY ===

void op_callback_sprite_visibility_store_id(void *addr) {

    param_sprite_vis.id = *(uint32_t *)addr;
}

void op_callback_sprite_visibility_store_toggle(void *addr) {

    param_sprite_vis.toggle = *(uint8_t *)addr;
}

void op_callback_sprite_visibility_conclude(void *addr) {

    hyper_sprite_visibility_params_t *p_sp_vis = (addr == NULL) ? &param_sprite_vis : (hyper_sprite_visibility_params_t *)addr;
    printf("OPERATION : setting the visibility of the sprite %d at %d...\n", p_sp_vis->id, p_sp_vis->toggle);

    sprites[p_sp_vis->id].toggle = p_sp_vis->toggle;
}


// === SPRITE POSITION ===

void op_callback_sprite_position_store_id(void *addr) {

    param_sprite_pos.id = *(uint32_t *)addr;
}

void op_callback_sprite_position_store_x(void *addr) {

    param_sprite_pos.x = *(uint32_t *)addr;
}

void op_callback_sprite_position_store_y(void *addr) {

    param_sprite_pos.y = *(uint32_t *)addr;
}

void op_callback_sprite_position_conclude(void *addr) {

    hyper_sprite_position_params_t *p_sp_pos = (addr == NULL) ? &param_sprite_pos : (hyper_sprite_position_params_t *)addr;
    printf("OPERATION : setting the position of the sprite %d at (%d, %d)...\n", p_sp_pos->id, p_sp_pos->x, p_sp_pos->y);

    sprites[p_sp_pos->id].x = p_sp_pos->x;
    sprites[p_sp_pos->id].y = p_sp_pos->y;
}

// === KEYBOARD ===

void op_keyboard_pv_send_code(void *shared_buf, uint32_t key) {

    printf("OPERATION : updating %d (%c) as the keycode of the shared buffer...\n", key, (char)key);

    hyper_keyboard_params_t *p_keyboard = (hyper_keyboard_params_t *)shared_buf;
    p_keyboard->key = key;
}
