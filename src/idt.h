// SPDX-LICENSE-Identifier: GPL-3.0

/*
 * Copyright Rusin Danilo <rusindanilo@gmail.com> VoltagedDebunked
 */

#ifndef IDT_H
#define IDT_H

void idt_set_entry (unsigned char num, unsigned long long base,
                    unsigned short sel, unsigned char flags);
void init_idt ();

#endif
