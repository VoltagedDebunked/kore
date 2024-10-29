// SPDX-LICENSE-Identifier: GPL-3.0

/*
 * Copyright Rusin Danilo <rusindanilo@gmail.com> VoltagedDebunked
 */

#include "idt.h"
#include "io.h"
#include <stdint.h>

struct idt_entry
{
  unsigned short base_low;
  unsigned short sel;
  unsigned char ist; // Interrupt Stack Table
  unsigned char flags;
  unsigned short base_middle;
  unsigned int base_high;
  unsigned int reserved;
} __attribute__ ((packed));

struct idt_ptr
{
  unsigned short limit;
  uintptr_t base; // Use uintptr_t for pointer conversion
} __attribute__ ((packed));

struct idt_entry idt[256];
struct idt_ptr idt_p;

extern void timer_handler ();
extern void keyboard_handler ();

void
idt_set_entry (unsigned char num, unsigned long long base, unsigned short sel,
               unsigned char flags)
{
  idt[num].base_low = (base & 0xFFFF);
  idt[num].base_middle = (base >> 16) & 0xFFFF;
  idt[num].base_high = (base >> 32) & 0xFFFFFFFF;
  idt[num].sel = sel;
  idt[num].ist = 0;
  idt[num].flags = flags;
  idt[num].reserved = 0;
}

void
init_idt ()
{
  idt_p.limit = (sizeof (struct idt_entry) * 256) - 1;
  idt_p.base = (uintptr_t)&idt;

  for (int i = 0; i < 256; i++)
    {
      idt_set_entry (i, 0, 0, 0);
    }

  // Set up timer interrupt handler (IRQ 0)
  idt_set_entry (32, (unsigned long long)timer_handler, 0x08, 0x8E);

  // Set up keyboard interrupt handler (IRQ 1)
  idt_set_entry (33, (unsigned long long)keyboard_handler, 0x08, 0x8E);

  asm volatile ("lidt (%0)" : : "r"(&idt_p));
}
