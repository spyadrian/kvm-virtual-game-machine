#include "ide.h"

#include <stdint.h>
#include "guest/pmio.h"
#include "shared/ide_regs.h"

// --- DEFINE ---

// ...

// --- STATIC FUNCTION ---

/**
 * Wait for the disk drive to be ready.
 */
static void wait_drive() {

    // 0xC0 = 0b1100'0000, 0x40 = 0b0100'0000
    while ((inb(REG_IDE_ST) & 0xC0) != 0x40);
}

/**
 * Prepare the disk drive for read/write at the specified sector in 28-bit LBA mode.
 * @param sector the sector to read or write
 */
static void pio_prepare(uint32_t sector) {

    wait_drive();

    outb(REG_IDE_DATA+2, 1);                               // sector count
    outb(REG_IDE_DATA+3, sector & 0xff);                   // send bits 0-7 of LBA
    outb(REG_IDE_DATA+4, (sector >> 8) & 0xff);            // send bits 8-15 of LBA
    outb(REG_IDE_DATA+5, (sector >> 16) & 0xff);           // send bits 16-23 of LBA
    outb(REG_IDE_DATA+6, ((sector >> 24) & 0x0f) | 0xe0);  // send bits 24-27 of LBA + set LBA mode; 0xe0 = 11100000b;
}

// --- FUNCTION ---

void ide_phys_write_sector(uint32_t sector, void *data) {

    pio_prepare(sector);

    outb(REG_IDE_CMD, 0x30);  // command port: write with retry
    wait_drive();

    uint16_t *d = (uint16_t *)data;
    for (int i = 0; i < MAX_LOOP_IDE_DATA; i++) {

        outw(REG_IDE_DATA, *d);
        d++;
    }
}

