#include "ide.h"

#include "guest/pmio.h"
#include "guest/utils.h"
#include "shared/hypercall_params.h"

// --- DEFINE ---

// ...

// --- FUNCTION ---

void ide_pv_write_sector(uint32_t sector_idx, void *data) {

    hyper_ide_params_t param_ide;
    param_ide.sector_idx = sector_idx;
    param_ide.data = (uint64_t)data;
    memcpy((void *)HYPERCALL_SHARED_ADDR, (void *)&param_ide, sizeof(param_ide));

    outb(HYPERCALL_PMIO_ADDR, HYPERCALL_CODE_IDE);
}

