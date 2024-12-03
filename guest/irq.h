#ifndef _IRQ_H_
#define _IRQ_H_

#include <stdint.h>

// --- DEFINE ---

#define IRQ_FIRST    0
#define IRQ_LAST     15

#define IRQ_TIMER       0
#define IRQ_KEYBOARD    1

typedef struct {

    void (*func)(void);   // pointer to handler function
    char name[64];        // associate a name to the function (for debugging purposes)
} handler_t;

// --- FUNCTION ---

// Initializes the array of IRQ handlers.
void irq_init();

// Installs a handler for the given IRQ.
// The irq parameter must be in the range [0,15] inclusive.
void irq_install_handler(uint32_t irq, handler_t handler);

// Retrieves the handler for a given IRQ.
// The irq parameter must be in the range [0,15] inclusive.
// Returns NULL if there is no handler for the given IRQ.
handler_t *irq_get_handler(uint32_t irq);

#endif // _IRQ_H_
