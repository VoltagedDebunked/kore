// SPDX-LICENSE-Identifier: GPL-3.0

/*
 * Copyright Rusin Danilo <rusindanilo@gmail.com> VoltagedDebunked
 */

#include "io.h"
#include <stdint.h>

#define SERIAL_PORT 0x3F8
#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define WHITE_ON_BLACK 0x0F
#define BLACK_ON_WHITE 0xF0

unsigned short *vga_buffer = (unsigned short *)VGA_MEMORY;
int cursor_x = 0;
int cursor_y = 0;

// Function prototypes
void clear_screen ();
void update_cursor ();
void echo_input ();
unsigned char read_serial ();
void write_serial (unsigned char data);
void init_memory_mapped_io ();

void
outb (unsigned short port, unsigned char data)
{
  asm volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

unsigned char
inb (unsigned short port)
{
  unsigned char result;
  asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
  return result;
}

unsigned char
inb_asm (
    unsigned short port) //  This is a stupid duplicate of inb, i had
                         // to rename it so its aviable in assembly. fuck...
{
  unsigned char result;
  asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
  return result;
}

void
outw (unsigned short port, unsigned short data)
{
  __asm__ volatile ("outw %0, %1" : : "a"(data), "Nd"(port));
}

unsigned short
inw (unsigned short port)
{
  unsigned short ret;
  __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

void
init_io ()
{
  outb (SERIAL_PORT + 1, 0x00); // Disable all interrupts
  outb (SERIAL_PORT + 3, 0x80); // Enable DLAB (set baud rate)
  outb (SERIAL_PORT + 0, 0x03); // Set baud rate to 38400 (lo byte)
  outb (SERIAL_PORT + 1, 0x00); // (hi byte)
  outb (SERIAL_PORT + 3, 0x03); // 8 bits, no parity, one stop bit
  outb (SERIAL_PORT + 2,
        0xC7); // Enable FIFO, clear them, with 14-byte threshold
  outb (SERIAL_PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
  clear_screen ();              // Clear screen at startup
}

void
clear_screen ()
{
  for (int y = 0; y < VGA_HEIGHT; y++)
    {
      for (int x = 0; x < VGA_WIDTH; x++)
        {
          vga_buffer[y * VGA_WIDTH + x]
              = (WHITE_ON_BLACK << 8) | ' '; // White space
        }
    }
  cursor_x = 0;
  cursor_y = 0;
  update_cursor ();
}

void
update_cursor ()
{
  unsigned short pos = cursor_y * VGA_WIDTH + cursor_x;
  outb (0x3D4, 0x0F); // Set cursor location high byte
  outb (0x3D5, (pos >> 8) & 0xFF);
  outb (0x3D4, 0x0E); // Set cursor location low byte
  outb (0x3D5, pos & 0xFF);
}

void
print_char (char c)
{
  if (c == '\n')
    {
      cursor_x = 0;
      cursor_y++;
    }
  else if (c == '\b')
    {
      if (cursor_x > 0)
        {
          cursor_x--;
        }
      vga_buffer[cursor_y * VGA_WIDTH + cursor_x]
          = (WHITE_ON_BLACK << 8) | ' ';
    }
  else
    {
      vga_buffer[cursor_y * VGA_WIDTH + cursor_x]
          = (WHITE_ON_BLACK << 8) | c; // White character
      cursor_x++;
    }

  if (cursor_x >= VGA_WIDTH)
    {
      cursor_x = 0;
      cursor_y++;
    }

  if (cursor_y >= VGA_HEIGHT)
    {
      cursor_y = VGA_HEIGHT - 1; // Keep in bounds
      for (int y = 0; y < VGA_HEIGHT - 1; y++)
        {
          for (int x = 0; x < VGA_WIDTH; x++)
            {
              vga_buffer[y * VGA_WIDTH + x]
                  = vga_buffer[(y + 1) * VGA_WIDTH + x]; // Scroll up
            }
        }
      for (int x = 0; x < VGA_WIDTH; x++)
        {
          vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x]
              = (WHITE_ON_BLACK << 8) | ' '; // Clear last line
        }
    }

  update_cursor ();
}

#define VGA_BUFFER (0xB8000)

void
print_string (const char *str)
{
  char *vga_buffer = (char *)VGA_BUFFER;
  while (*str)
    {
      *vga_buffer++ = *str++;
      *vga_buffer++ = 0x07; // Light grey on black
    }
}

void
echo_input ()
{
  while (1)
    {
      unsigned char c = read_serial (); // Wait for input
      print_char (c);                   // Echo character back to screen
      write_serial (c); // Send character back through serial port
    }
}

void
delay (int ms)
{
  for (volatile int i = 0; i < ms * 1000; i++)
    ;
}

unsigned char
read_serial ()
{
  while ((inb (SERIAL_PORT + 5) & 1) == 0)
    ; // Wait for data
  return inb (SERIAL_PORT);
}

void
write_serial (unsigned char data)
{
  while ((inb (SERIAL_PORT + 5) & 0x20) == 0)
    ; // Wait for the Transmitter Holding Register to be empty
  outb (SERIAL_PORT, data);
}

void
init_memory_mapped_io ()
{
  unsigned int base_address = 0x80000000; // Example base address
  write_memory (base_address, 0xFF);      // Write some value to the device
}

void
write_memory (uintptr_t address, unsigned char value)
{
  *(volatile unsigned char *)address = value; // Memory-mapped I/O
}

unsigned char
read_memory (uintptr_t address)
{
  return *(volatile unsigned char *)address; // Read memory-mapped I/O
}
