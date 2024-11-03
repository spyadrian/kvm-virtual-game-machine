#ifndef _IDE_H_
#define _IDE_H_

#include <stdint.h>

// --- DEFINE ---

// ...

// --- FUNCTION ---

/**
 * Read a sector from the first disk.
 * @param first sector to read (0-indexed)
 * @param dst address to store to read data
 * Based on the assembly code at http://wiki.osdev.org/ATA_read/write_sectors
 */
void ide_phys_write_sector(uint32_t sector_idx, void *data);

void ide_pv_write_sector(uint32_t sector_idx, void *data);

#endif // _IDE_H_
