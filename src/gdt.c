// SPDX-LICENSE-Identifier: GPL-3.0
/*
 * Copyright Rusin Danilo <rusindanilo@gmail.com> VoltagedDebunked
 */

#include "gdt.h"

// System segment types
#define SYSTEM_TSS_AVAILABLE 0x9
#define SYSTEM_TSS_BUSY 0xB

struct gdt_entry
{
  unsigned short limit_low;
  unsigned short base_low;
  unsigned char base_middle;
  unsigned char access;
  unsigned char granularity;
  unsigned char base_high;
  unsigned int base_upper; // Upper 32 bits for 64-bit base address
  unsigned int reserved;   // Must be zero
} __attribute__ ((packed));

struct tss_entry
{
  unsigned int reserved0;
  unsigned long long rsp[3]; // Stack pointers for privilege levels 0-2
  unsigned long long reserved1;
  unsigned long long ist[7]; // Interrupt Stack Table pointers
  unsigned long long reserved2;
  unsigned short reserved3;
  unsigned short iopb_offset; // I/O Permission Bitmap offset
} __attribute__ ((packed));

struct gdt_ptr
{
  unsigned short limit;
  unsigned long long base;
} __attribute__ ((packed));

// We'll need more entries for 64-bit mode
// Null + Code + Data + User Code + User Data + TSS
struct gdt_entry gdt[6];
struct gdt_ptr gdt_p;
struct tss_entry tss;

/**
 * @brief Sets up a Global Descriptor Table (GDT) entry for 64-bit mode.
 *
 * This function configures a single entry in the GDT with the specified
 * parameters. It supports both system segments (like TSS) and code/data
 * segments with full 64-bit base address support.
 *
 * @param num The index of the GDT entry to be set
 * @param base The base address of the segment (64-bit)
 * @param limit The limit (size) of the segment
 * @param access The access byte specifying segment properties
 * @param granularity The granularity byte specifying additional properties
 */
static void
gdt_set_entry (int num, unsigned long long base, unsigned long limit,
               unsigned char access, unsigned char granularity)
{
  gdt[num].base_low = (base & 0xFFFF);
  gdt[num].base_middle = (base >> 16) & 0xFF;
  gdt[num].base_high = (base >> 24) & 0xFF;
  gdt[num].base_upper = (base >> 32) & 0xFFFFFFFF;
  gdt[num].limit_low = (limit & 0xFFFF);
  gdt[num].granularity = (limit >> 16) & 0x0F;
  gdt[num].granularity |= granularity & 0xF0;
  gdt[num].access = access;
  gdt[num].reserved = 0;
}

/**
 * @brief Sets up a Task State Segment (TSS) entry in the GDT.
 *
 * This function creates a TSS descriptor in the GDT, which is required
 * for 64-bit mode operation. The TSS holds information about privilege
 * level stacks and interrupt stacks.
 *
 * @param num The index of the GDT entry to be used for TSS
 * @param base The base address of the TSS structure
 * @param limit The size of the TSS structure
 */
static void
gdt_set_tss (int num, unsigned long long base, unsigned long limit)
{
  // Base address
  gdt[num].base_low = base & 0xFFFF;
  gdt[num].base_middle = (base >> 16) & 0xFF;
  gdt[num].base_high = (base >> 24) & 0xFF;
  gdt[num].base_upper = (base >> 32) & 0xFFFFFFFF;

  // Limit (size of TSS)
  gdt[num].limit_low = limit & 0xFFFF;
  gdt[num].granularity = (limit >> 16) & 0x0F;

  // Access byte - Present, Ring 0, System Segment, Type (9 for available TSS)
  gdt[num].access = 0x80 | (0 << 5) | SYSTEM_TSS_AVAILABLE;

  // Granularity byte - 4KB blocks, 64-bit TSS
  gdt[num].granularity |= 0x00; // No 4KB granularity for TSS

  gdt[num].reserved = 0;
}

/**
 * @brief Initializes the TSS structure.
 *
 * Sets up the Task State Segment with appropriate stack pointers and
 * interrupt stack table entries. This is required for 64-bit operation.
 */
static void
init_tss (void)
{
  // Clear TSS structure
  for (int i = 0; i < sizeof (tss); i++)
    {
      ((unsigned char *)&tss)[i] = 0;
    }

  // Set up privilege level 0 stack (kernel stack)
  tss.rsp[0] = 0x200000; // Example stack address, adjust as needed

  // Set up Interrupt Stack Table (if needed)
  // tss.ist[0] = 0x100000;  // Example IST address

  // Set I/O Permission Bitmap offset to beyond TSS limit
  tss.iopb_offset = sizeof (tss);
}

/**
 * @brief Initializes the 64-bit Global Descriptor Table.
 *
 * Sets up a GDT suitable for long mode (64-bit) operation, including:
 * - Null descriptor
 * - Kernel code segment
 * - Kernel data segment
 * - User code segment
 * - User data segment
 * - Task State Segment
 */
void
init_gdt (void)
{
  gdt_p.limit = (sizeof (struct gdt_entry) * 6) - 1;
  gdt_p.base = (unsigned long long)&gdt;

  // Null segment
  gdt_set_entry (0, 0, 0, 0, 0);

  // Kernel code segment - 64-bit
  // Access: Present, Ring 0, Code Segment, Executable, Read
  gdt_set_entry (1, 0, 0x000FFFFF, 0x9A | 0x20, 0xA0);

  // Kernel data segment
  // Access: Present, Ring 0, Data Segment, Read/Write
  gdt_set_entry (2, 0, 0x000FFFFF, 0x92, 0xA0);

  // User code segment - 64-bit
  // Access: Present, Ring 3, Code Segment, Executable, Read
  gdt_set_entry (3, 0, 0x000FFFFF, 0xFA | 0x20, 0xA0);

  // User data segment
  // Access: Present, Ring 3, Data Segment, Read/Write
  gdt_set_entry (4, 0, 0x000FFFFF, 0xF2, 0xA0);

  // Initialize TSS
  init_tss ();

  // TSS segment
  gdt_set_tss (5, (unsigned long long)&tss, sizeof (tss) - 1);

  // Load GDT
  asm volatile ("lgdt (%0)" : : "r"(&gdt_p));

  // Load TSS
  asm volatile ("ltr %%ax" : : "a"(0x28)); // 0x28 = TSS segment selector
}
