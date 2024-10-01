#ifndef _HYPERCALL_PARAMS_H_
#define _HYPERCALL_PARAMS_H_

#include <stdint.h>

typedef struct {
    uint32_t us;  // delay in micro-seconds
} __attribute__((packed)) hyper_timer_sleep_params_t;

#endif
