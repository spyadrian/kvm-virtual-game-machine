#ifndef _IDE_REGS_H_
#define _IDE_REGS_H_

#define IDE_STATUS_REG     0x1F7  // Status register on the primary bus (read-only)
#define IDE_CMD_REG        0x1F7  // Command register on the primary bus (write-only)
#define IDE_DATA_BASE_REG  0x1F0  // Base port on primary bus (write-only)

#define SECTOR_SIZE         512
#define MAX_SECTOR_NUM      ((1UL << 28) - 1)
#define MAX_LOOP_IDE_DATA   (SECTOR_SIZE / 2)

#endif
