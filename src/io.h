// SPDX-LICENSE-Identifier: GPL-3.0

/*
 * Copyright Rusin Danilo <rusindanilo@gmail.com> VoltagedDebunked
 */

#ifndef IO_H
#define IO_H

#include <stdint.h>

void init_io ();
void outb (unsigned short port, unsigned char data);
unsigned char inb (unsigned short port);
void outw (unsigned short port, unsigned short data);
unsigned short inw (unsigned short port);
void write_memory (uintptr_t address, unsigned char value);
unsigned char read_memory (uintptr_t address);
void print_string (const char *str);
void print_number (uint64_t num);
void print_hex (uint64_t num);

#endif
