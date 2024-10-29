#include "timer.h"

#include "guest/utils.h"
#include "guest/pmio.h"
#include "shared/hypercall_params.h"

// --- DEFINE ---

// ...

// --- FUNCTION ---

void timer_pv_wait(uint32_t us) {

    hyper_timer_sleep_params_t param_timer;
    param_timer.us = us;
    memcpy((void *)HYPERCALL_SHARED_ADDR, (void *)&param_timer, sizeof(param_timer));

    outb(HYPERCALL_PMIO_ADDR, HYPERCALL_CODE_TIMER);
}

