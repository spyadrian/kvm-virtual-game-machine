#ifndef _IDE_REGS_H_
#define _IDE_REGS_H_

#define REG_IDE_ST    0x1F7  // Status register on the primary bus (read-only)
#define REG_IDE_CMD   0x1F7  // Command register on the primary bus (write-only)
#define REG_IDE_DATA  0x1F0  // Base port on primary bus (write-only)

#define SECTOR_SIZE         512
#define MAX_SECTOR_NUM      ((1UL << 28) - 1)
#define MAX_LOOP_IDE_DATA   (SECTOR_SIZE / 2)

#endif
