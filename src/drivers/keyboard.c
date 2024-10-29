// SPDX-LICENSE-Identifier: GPL-3.0

/*
 * Copyright Rusin Danilo <rusindanilo@gmail.com> VoltagedDebunked
 */

#include "keyboard.h"
#include "../io.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_COMMAND_PORT 0x64

#define MAX_KEYS 256

static char keymap[MAX_KEYS] = { 0 };
static char shift_keymap[MAX_KEYS] = { 0 };
static unsigned char shift_pressed = 0;

void print_char (char c);

// Initialize the keyboard
void
init_keyboard ()
{
  outb (KEYBOARD_COMMAND_PORT, 0xAE); // Enable keyboard interrupts
}

// Keyboard interrupt callback
void
keyboard_callback ()
{
  unsigned char scancode = inb (KEYBOARD_DATA_PORT);

  if (scancode & 0x80)
    {
      // Handle key release if necessary
      if (scancode == 0x2A || scancode == 0x36)
        {
          shift_pressed = 0;
        }
      return;
    }

  int key = 0; // Change the type to int for comparison
  if (scancode < MAX_KEYS)
    {
      if (shift_pressed)
        {
          key = shift_keymap[scancode];
        }
      else
        {
          key = keymap[scancode];
        }

      if (key)
        {
          print_char (key);
        }

      if (scancode == 0x2A || scancode == 0x36)
        {
          shift_pressed = 1;
        }
    }
}

// Function to map scancodes to ASCII
void
setup_keymap ()
{
  // Basic key mapping (unshifted)
  keymap[0x1E] = 'a';
  keymap[0x30] = 'b';
  keymap[0x2E] = 'c';
  keymap[0x20] = ' ';
  keymap[0x15] = 'd';
  keymap[0x12] = 'e';
  keymap[0x21] = 'f';
  keymap[0x24] = 'g';
  keymap[0x13] = 'h';
  keymap[0x23] = 'i';
  keymap[0x17] = 'j';
  keymap[0x22] = 'k';
  keymap[0x19] = 'l';
  keymap[0x10] = 'm';
  keymap[0x14] = 'n';
  keymap[0x16] = 'o';
  keymap[0x1C] = 'p';
  keymap[0x1A] = 'q';
  keymap[0x1D] = 'r';
  keymap[0x18] = 's';
  keymap[0x11] = 't';
  keymap[0x14] = 'u';
  keymap[0x1F] = 'v';
  keymap[0x13] = 'w';
  keymap[0x12] = 'x';
  keymap[0x1B] = 'y';
  keymap[0x2C] = 'z';
  keymap[0x29] = '\n'; // Enter
  keymap[0x0E] = '\b'; // Backspace

  // Additional keys (unshifted)
  keymap[0x39] = ' '; // Space
  keymap[0x27] = ','; // Comma
  keymap[0x28] = '.'; // Period
  keymap[0x2D] = '-'; // Minus
  keymap[0x2E] = '='; // Equals

  // Shifted key mapping
  shift_keymap[0x1E] = 'A';
  shift_keymap[0x30] = 'B';
  shift_keymap[0x2E] = 'C';
  shift_keymap[0x20] = ' ';
  shift_keymap[0x15] = 'D';
  shift_keymap[0x12] = 'E';
  shift_keymap[0x21] = 'F';
  shift_keymap[0x24] = 'G';
  shift_keymap[0x13] = 'H';
  shift_keymap[0x23] = 'I';
  shift_keymap[0x17] = 'J';
  shift_keymap[0x22] = 'K';
  shift_keymap[0x19] = 'L';
  shift_keymap[0x10] = 'M';
  shift_keymap[0x14] = 'N';
  shift_keymap[0x16] = 'O';
  shift_keymap[0x1C] = 'P';
  shift_keymap[0x1A] = 'Q';
  shift_keymap[0x1D] = 'R';
  shift_keymap[0x18] = 'S';
  shift_keymap[0x11] = 'T';
  shift_keymap[0x14] = 'U';
  shift_keymap[0x1F] = 'V';
  shift_keymap[0x13] = 'W';
  shift_keymap[0x12] = 'X';
  shift_keymap[0x1B] = 'Y';
  shift_keymap[0x2C] = 'Z';
  shift_keymap[0x29] = '\n'; // Enter
  shift_keymap[0x0E] = '\b'; // Backspace

  // Additional keys (shifted)
  shift_keymap[0x39] = ' '; // Space
  shift_keymap[0x27] = '<'; // Comma
  shift_keymap[0x28] = '>'; // Period
  shift_keymap[0x2D] = '_'; // Minus
  shift_keymap[0x2E] = '+'; // Equals
}

// Call this function to initialize everything
void
initialize_keyboard_driver ()
{
  setup_keymap ();
  init_keyboard ();
}
