// SPDX-LICENSE-Identifier: GPL-3.0
/*
 * Copyright Rusin Danilo <rusindanilo@gmail.com> VoltagedDebunked
 */

#ifndef FIRMWARE_H
#define FIRMWARE_H

#include <stdbool.h>
#include <stdint.h>

// Memory types
typedef enum
{
  MEMORY_FREE = 0,
  MEMORY_RESERVED,
  MEMORY_ACPI_RECLAIM,
  MEMORY_ACPI_NVS,
  MEMORY_BAD,
  MEMORY_KERNEL,
  MEMORY_MODULE
} memory_type_t;

// Memory map entry
typedef struct
{
  uint64_t base;
  uint64_t length;
  memory_type_t type;
  uint32_t attributes;
} memory_map_entry_t;

// Memory map
typedef struct
{
  memory_map_entry_t *entries;
  uint32_t entry_count;
} memory_map_t;

// Firmware info structure
typedef struct
{
  char *vendor;
  uint32_t version;
  uint64_t rsdp_address;
  memory_map_t memory_map;
} firmware_info_t;

// Function prototypes
bool firmware_init (void *system_table);
void firmware_exit (void);
bool firmware_get_memory_map (memory_map_t *map);
void firmware_print_info (void);
uint64_t firmware_get_memory_size (void);

#endif // FIRMWARE_H
