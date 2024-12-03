#include "idt.h"
#include "utils.h"
#include "descriptors.h"
#include "x86.h"
#include "irq.h"

#define GDT_KERNEL_CODE_SELECTOR 0x08

// Structure of an IDT descriptor. There are 3 types of descriptors:
// task-gates, interrupt-gates, trap-gates.
// See 5.11 of Intel 64 & IA32 architectures software developer's manual for more details.
// For task gates, offset must be 0.
typedef struct idt_entry_st {
    uint16_t offset15_0;   // only used by trap and interrupt gates
    uint16_t selector;     // segment selector for trap and interrupt gates; TSS segment
                           // selector for task gates
    uint16_t reserved : 8;
    uint16_t type : 5;
    uint16_t dpl : 2;
    uint16_t p : 1;
    uint16_t offset31_16;  // only used by trap and interrupt gates
} __attribute__((packed)) idt_entry_t;

// Structure describing a pointer to the IDT gate table.
// This format is required by the lidt instruction.
typedef struct idt_ptr_st {
    uint16_t limit;   // Limit of the table (ie. its size)
    uint32_t base;    // Address of the first entry
} __attribute__((packed)) idt_ptr_t;

// Gates table
static idt_entry_t idt[256];
static idt_ptr_t   idt_ptr;

// Defined in idt_asm.s
void idt_load(idt_ptr_t *idt_ptr);

// Build and return an IDT entry.
// selector is the code segment selector to access the ISR
// offset is the address of the ISR (for task gates, offset must be 0)
// type indicates the IDT entry type
// dpl is the privilege level required to call the associated ISR
static idt_entry_t idt_build_entry(uint16_t selector, uint32_t offset, uint8_t type, uint8_t dpl) {
    idt_entry_t entry;
    entry.offset15_0 = offset & 0xffff;
    entry.selector = selector;
    entry.reserved = 0;
    entry.type = type;
    entry.dpl = dpl;
    entry.p = 1;
    entry.offset31_16 = (offset >> 16) & 0xffff;
    return entry;
}

// Low level handler defined in idt_asm.s
void _irq0();
void _irq1();

// IRQ handler
void irq_handler(uint32_t irq_number) {

    handler_t *hd = irq_get_handler(irq_number);
    if (hd != 0) hd->func();
}

void idt_init() {
    irq_init();

    idt_ptr.limit = sizeof(idt)-1;
    idt_ptr.base  = (uint32_t)&idt;

    // Clear up all entries
    memset(idt, 0, sizeof(idt));

    idt[0] = idt_build_entry(GDT_KERNEL_CODE_SELECTOR, (uint32_t)_irq0, TYPE_INTERRUPT_GATE, DPL_KERNEL);
    idt[1] = idt_build_entry(GDT_KERNEL_CODE_SELECTOR, (uint32_t)_irq1, TYPE_INTERRUPT_GATE, DPL_KERNEL);

    idt_load(&idt_ptr);
}
