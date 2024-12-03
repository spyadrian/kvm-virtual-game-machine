#include "irq.h"

#include "utils.h"
#include <stdint.h>

// --- DEFINE ---

#define IRQ_COUNT    (IRQ_LAST-IRQ_FIRST + 1)

static handler_t irq_handlers[IRQ_COUNT];

// --- FUNCTION ---

void irq_init() {

    memset(irq_handlers, 0, sizeof(irq_handlers));
}

void irq_install_handler(uint32_t irq, handler_t handler) {

    irq_handlers[irq] = handler;
}

handler_t *irq_get_handler(uint32_t irq) {

    return &irq_handlers[irq];
}

