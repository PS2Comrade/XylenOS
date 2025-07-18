#ifndef ATA_H
#define ATA_H

#include "../kernel/types.h"

int detect_hdd(void);
int ata_read_sector(uint32_t lba, uint8_t* buf);
int ata_write_sector(uint32_t lba, const uint8_t* buf);

#endif
