#ifndef _HYPERCALL_PARAMS_H_
#define _HYPERCALL_PARAMS_H_

#include <stdint.h>

// --- DEFINE ---

#define HYPERCALL_SHARED_ADDR 0xC0000000
#define HYPERCALL_BUFF_SIZE 4096

// --- SHARED STRUCT ---

typedef struct {

    uint64_t msg;    // address of the message
} __attribute__((packed)) hyper_virtual_console_params_t;

typedef struct {

    uint32_t us;    // delay in micro-seconds
} __attribute__((packed)) hyper_timer_sleep_params_t;

// --- FUNCTION ---

// ...

#endif
