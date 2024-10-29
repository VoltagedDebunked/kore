// SPDX-LICENSE-Identifier: GPL-3.0

/*
 * Copyright Rusin Danilo <rusindanilo@gmail.com> VoltagedDebunked
 */

#include "disk.h"
#include "../io.h"

void
init_disk ()
{
  // Initialization code for the disk
}

/**
 * @brief Reads a sector from the disk using Programmed Input/Output (PIO).
 *
 * This function reads a single sector of data from the disk at the specified
 * Logical Block Address (LBA). It uses the Intel 8230x Programmable
 * Input/Output (PIO) mode to communicate with the disk controller.
 *
 * @param lba The Logical Block Address (LBA) of the sector to read.
 * @param buffer A pointer to the buffer where the read data will be stored.
 *               Must be at least SECTOR_SIZE bytes in size.
 *
 * @return 0 on success, or a negative value on error.
 */
int
read_sector (uint64_t lba, void *buffer)
{
  // Assume using PIO to read a sector from disk
  outb (0x1F6, 0xE0); // Set up master drive (0xE0 for the first drive)
  outb (0x1F1, (uint8_t)((lba >> 24) & 0xFF)); // High byte of LBA
  outb (0x1F2, (uint8_t)(SECTOR_SIZE / 512));  // Number of sectors to read (1)
  outb (0x1F3, (uint8_t)(lba & 0xFF));         // Low byte of LBA
  outb (0x1F4, (uint8_t)((lba >> 8) & 0xFF));  // Mid byte of LBA
  outb (0x1F5, (uint8_t)((lba >> 16) & 0xFF)); // High byte of LBA
  outb (0x1F7, 0x20); // Issue the command (READ SECTOR)

  // Wait for the disk to be ready
  while ((inb (0x1F7) & 0xC0) == 0)
    {
    }

  // Read the sector data
  for (int i = 0; i < SECTOR_SIZE / 2; i++)
    {
      ((uint16_t *)buffer)[i] = inw (0x1F0);
    }

  return 0; // Return 0 for success
}

/**
 * @brief Writes a sector to the disk using Programmed Input/Output (PIO).
 *
 * This function writes a single sector of data to the disk at the specified
 * Logical Block Address (LBA). It uses the Intel 8230x Programmable
 * Input/Output (PIO) mode to communicate with the disk controller.
 *
 * @param lba The Logical Block Address (LBA) of the sector to write.
 * @param buffer A pointer to the buffer containing the data to be written.
 *
 * @return 0 on success, or a negative value on error.
 */
int
write_sector (uint64_t lba, const void *buffer)
{
  // Assume using PIO to write a sector to disk
  outb (0x1F6, 0xE0); // Set up master drive (0xE0 for the first drive)
  outb (0x1F1, (uint8_t)((lba >> 24) & 0xFF)); // High byte of LBA
  outb (0x1F2, (uint8_t)(SECTOR_SIZE / 512)); // Number of sectors to write (1)
  outb (0x1F3, (uint8_t)(lba & 0xFF));        // Low byte of LBA
  outb (0x1F4, (uint8_t)((lba >> 8) & 0xFF)); // Mid byte of LBA
  outb (0x1F5, (uint8_t)((lba >> 16) & 0xFF)); // High byte of LBA
  outb (0x1F7, 0x30); // Issue the command (WRITE SECTOR)

  // Wait for the disk to be ready
  while ((inb (0x1F7) & 0xC0) == 0)
    {
    }

  // Write the sector data
  for (int i = 0; i < SECTOR_SIZE / 2; i++)
    {
      outw (0x1F0, ((const uint16_t *)buffer)[i]);
    }

  return 0; // Return 0 for success
}

void
disk_info ()
{
  // Optionally implement function to display disk info
}
