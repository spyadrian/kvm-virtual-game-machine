#ifndef _IDE_H_
#define _IDE_H_

#define SECTOR_SIZE 512

// Write a sector.
// Implement a device driver for the true physical hardware.
void ide_write_sector_phys(int sector_idx, void *data);

#endif
