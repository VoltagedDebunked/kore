// SPDX-LICENSE-Identifier: GPL-3.0

/*
 * Copyright Rusin Danilo <rusindanilo@gmail.com> VoltagedDebunked
 */

#include "timer.h"
#include "../io.h"

#define TIMER_COMMAND 0x43
#define TIMER_CHANNEL 0x40
#define PIT_FREQUENCY 1193180

void
init_timer (int frequency)
{
  int divisor = PIT_FREQUENCY / frequency; // Why the fuck are we using this
  outb (TIMER_COMMAND, 0x36);
  outb (TIMER_CHANNEL, divisor & 0xFF);
  outb (TIMER_CHANNEL, (divisor >> 8) & 0xFF);
}
