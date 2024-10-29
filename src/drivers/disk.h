// SPDX-LICENSE-Identifier: GPL-3.0

/*
 * Copyright Rusin Danilo <rusindanilo@gmail.com> VoltagedDebunked
 */

#ifndef DISK_H
#define DISK_H

#include <stdint.h>

#define SECTOR_SIZE 512

void init_disk ();
int read_sector (uint64_t lba, void *buffer);
int write_sector (uint64_t lba, const void *buffer);
void disk_info ();

#endif
