#include "console.h"

#include "guest/utils.h"
#include "guest/pmio.h"
#include "shared/hypercall_params.h"
#include <stdint.h>

// --- DEFINE ---

// ...

// --- FUNCTION ---

void console_pv_send(char *str) {

    hyper_virtual_console_params_t param_console;
    param_console.msg = (uint64_t)((uint32_t)str);
    memcpy((void *)HYPERCALL_SHARED_ADDR, (void *)&param_console, sizeof(param_console));

    outb(HYPERCALL_PMIO_ADDR, HYPERCALL_CODE_CONSOLE);
}

