// SPDX-LICENSE-Identifier: GPL-3.0

/*
 * Copyright Rusin Danilo <rusindanilo@gmail.com> VoltagedDebunked
 */

#include "gdt.h"

struct gdt_entry
{
  unsigned short limit_low;
  unsigned short base_low;
  unsigned char base_middle;
  unsigned char access;
  unsigned char granularity;
  unsigned char base_high;
  unsigned int base_upper; // 64-bit support
  unsigned int reserved;   // Must be zero
} __attribute__ ((packed));

struct gdt_ptr
{
  unsigned short limit;
  unsigned long long base;
} __attribute__ ((packed));

struct gdt_entry gdt[3];
struct gdt_ptr gdt_p;

/**
 * @brief Sets up a Global Descriptor Table (GDT) entry.
 *
 * This function configures a single entry in the GDT with the specified
 * parameters. It sets up the base address, limit, access flags, and
 * granularity for the segment.
 *
 * @param num The index of the GDT entry to be set.
 * @param base The base address of the segment (64-bit).
 * @param limit The limit (size) of the segment.
 * @param access The access byte specifying segment properties.
 * @param granularity The granularity byte specifying additional segment
 * properties.
 *
 * @return This function does not return a value.
 */
static void
gdt_set_entry (int num, unsigned long long base, unsigned long limit,
               unsigned char access, unsigned char granularity)
{
  gdt[num].base_low = (base & 0xFFFF);
  gdt[num].base_middle = (base >> 16) & 0xFF;
  gdt[num].base_high = (base >> 24) & 0xFF;
  gdt[num].base_upper
      = (base >> 32) & 0xFFFFFFFF; // Set upper 32-bits for 64-bit support
  gdt[num].limit_low = (limit & 0xFFFF);
  gdt[num].granularity = (limit >> 16) & 0x0F;
  gdt[num].granularity |= granularity & 0xF0;
  gdt[num].access = access;
  gdt[num].reserved = 0; // Reserved field must be zero
}

void
init_gdt ()
{
  gdt_p.limit = (sizeof (struct gdt_entry) * 3) - 1;
  gdt_p.base = (unsigned long long)&gdt;

  gdt_set_entry (0, 0, 0, 0, 0);                // Null segment
  gdt_set_entry (1, 0, 0x000FFFFF, 0x9A, 0xA0); // Code segment
  gdt_set_entry (2, 0, 0x000FFFFF, 0x92, 0xA0); // Data segment

  asm volatile ("lgdt (%0)" : : "r"(&gdt_p));
}
