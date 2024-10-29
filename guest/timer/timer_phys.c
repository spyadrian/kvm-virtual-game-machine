#include "timer.h"

#include "guest/pmio.h"
#include "shared/hypercall_params.h"

// --- DEFINE ---

// ...

// --- FUNCTION ---

void timer_phys_wait(uint32_t us) {

    outw(REG_TIMER_CMD, 20);
    outd(REG_TIMER_DATA, us);
}

