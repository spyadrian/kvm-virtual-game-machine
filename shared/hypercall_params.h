#ifndef _HYPERCALL_PARAMS_H_
#define _HYPERCALL_PARAMS_H_

#include <stdint.h>

// --- DEFINE ---

#define HYPERCALL_PMIO_ADDR   0xABBA

#define HYPERCALL_SHARED_ADDR   0xC0000000
#define HYPERCALL_BUFF_SIZE     4096

#define HYPERCALL_CODE_CONSOLE      1
#define HYPERCALL_CODE_TIMER        2
#define HYPERCALL_CODE_GFX_INIT     3
#define HYPERCALL_CODE_IDE          4
#define HYPERCALL_CODE_SPRITE_INIT  5
#define HYPERCALL_CODE_SPRITE_VISI  6
#define HYPERCALL_CODE_SPRITE_POS   7

#define REG_TIMER_CMD  0x43
#define REG_TIMER_DATA 0x40

#define REG_GFX_INIT_ST   0x4E700000
#define REG_GFX_INIT_CMD  0x4E700100
#define REG_GFX_INIT_DATA 0x4E700200

// --- SHARED STRUCT ---

typedef struct {

    uint64_t msg;    // address of the message
} __attribute__((packed)) hyper_virtual_console_params_t;

typedef struct {

    uint32_t us;    // delay in micro-seconds
} __attribute__((packed)) hyper_timer_sleep_params_t;

typedef struct {

    uint32_t width;   // horizontal resolution in pixels
    uint32_t height;  // vertical resolution in pixels 
} __attribute__((packed)) hyper_init_gfx_params_t;

typedef struct {

    uint32_t sector_idx;  // sector index ( or number )
    uint64_t data;        // address of data to write (512 bytes)
} __attribute__((packed)) hyper_ide_params_t;

typedef struct {

    uint8_t id;         // sprite identifier
    uint64_t data;      // data address
    uint32_t width;     // width in pixels
    uint32_t height;    // height in pixels
} __attribute__((packed)) hyper_sprite_init_params_t;

typedef struct {

    uint8_t id;         // sprite identifier
    uint8_t toggle;     // visibility (1 = visible , 0 = not visible )
} __attribute__((packed)) hyper_sprite_visibility_params_t;

typedef struct {

    uint8_t id;         // sprite identifier
    uint32_t x;         // x position
    uint32_t y;         // y position
} __attribute__((packed)) hyper_sprite_position_params_t;

// --- FUNCTION ---

// ...

#endif // _HYPERCALL_PARAMS_H_
