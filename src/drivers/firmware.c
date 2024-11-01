// SPDX-LICENSE-Identifier: GPL-3.0
/*
 * Copyright Rusin Danilo <rusindanilo@gmail.com> VoltagedDebunked
 */

#include "firmware.h"
#include "../io.h"

static firmware_info_t firmware_info = { 0 };
static memory_map_entry_t default_memory_map[1] = {
  { 0x100000, 0x1000000, MEMORY_FREE, 0 } // Default 16MB memory map entry
};

bool
firmware_init (void *system_table)
{
  // For now, just set up a basic memory map
  firmware_info.vendor = "Basic Firmware";
  firmware_info.version = 1;
  firmware_info.rsdp_address = 0;
  firmware_info.memory_map.entries = default_memory_map;
  firmware_info.memory_map.entry_count = 1;

  return true;
}

void
firmware_exit (void)
{
  // Nothing to do yet
}

bool
firmware_get_memory_map (memory_map_t *map)
{
  if (!map)
    return false;

  map->entries = firmware_info.memory_map.entries;
  map->entry_count = firmware_info.memory_map.entry_count;
  return true;
}

void
firmware_print_info (void)
{
  print_string ("Firmware Vendor: ");
  print_string (firmware_info.vendor);
  print_string ("\nFirmware Version: ");
  // Assuming you have these functions in io.c
  print_string ("\nMemory Map:\n");

  for (uint32_t i = 0; i < firmware_info.memory_map.entry_count; i++)
    {
      memory_map_entry_t *entry = &firmware_info.memory_map.entries[i];
      print_string ("Base: ");
      // You'll need to implement these print functions
      print_string ("\n");
    }
}

uint64_t
firmware_get_memory_size (void)
{
  uint64_t total_size = 0;
  for (uint32_t i = 0; i < firmware_info.memory_map.entry_count; i++)
    {
      if (firmware_info.memory_map.entries[i].type == MEMORY_FREE)
        {
          total_size += firmware_info.memory_map.entries[i].length;
        }
    }
  return total_size;
}
