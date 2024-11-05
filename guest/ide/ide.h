#ifndef _IDE_H_
#define _IDE_H_

#include <stdint.h>

// --- DEFINE ---

// ...

// --- FUNCTION ---

/**
 * @brief Writes data to a specified sector on the IDE device using paravirtualization.
 *
 * Transmits a data block to a specified IDE sector in a paravirtualized manner, 
 * allowing the guest to write data more efficiently.
 *
 * @param sector_idx Index of the target sector on the IDE device.
 * @param data       Pointer to the data buffer to be written.
 */
void ide_pv_write_sector(uint32_t sector_idx, void *data);

/**
 * @brief Writes data to a specified sector on the IDE device using emulation.
 *
 * Transfers a data block to a specific sector of the emulated IDE device, 
 * simulating a hardware IDE write operation.
 *
 * @param sector_idx Index of the target sector on the IDE device.
 * @param data       Pointer to the data buffer to be written.
 */
void ide_phys_write_sector(uint32_t sector_idx, void *data);

#endif // _IDE_H_
