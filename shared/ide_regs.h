#ifndef _IDE_REGS_H_
#define _IDE_REGS_H_

// Status register on the primary bus (read-only)
#define IDE_STATUS_REG     0x1F7

// Command register on the primary bus (write-only)
#define IDE_CMD_REG        0x1F7

// Base port on primary bus (write-only)
#define IDE_DATA_BASE_REG  0x1F0

#endif
