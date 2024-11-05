#ifndef _CONSOLE_H_
#define _CONSOLE_H_

// --- DEFINE ---

// ...

// --- FUNCTION ---

/**
 * @brief Sends a message to the virtual console using paravirtualization.
 *
 * Transfers a string message to the VMM console via a paravirtualized hypercall,
 * allowing the guest to output text to the VMM console.
 *
 * @param str Pointer to a null-terminated string containing the message to be sent.
 */
void console_pv_send(char *str);

#endif // _CONSOLE_H_
