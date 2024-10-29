// SPDX-LICENSE-Identifier: GPL-3.0
/*
 * Copyright Rusin Danilo <rusindanilo@gmail.com> VoltagedDebunked
 */

#include "drivers/disk.h"
#include "drivers/keyboard.h"
#include "drivers/timer.h"
#include "gdt.h"
#include "idt.h"
#include "io.h"
#include "memory.h"

#define MAX_PROCESSES 32
#define STACK_SIZE 4096
#define PROCESS_READY 1
#define PROCESS_BLOCKED 0

// Forward declarations
void init_process (void);
extern void register_interrupt_handler (uint64_t n, void (*handler) (void));

// Process Control Block structure
typedef struct
{
  uint64_t pid;
  uint64_t rsp;
  uint64_t rbp;
  uint64_t state;
  uint64_t time_slice;
  void *stack;
} PCB;

// System state
static PCB process_table[MAX_PROCESSES];
static uint64_t current_pid = 0;
static uint64_t next_pid = 1;

// Memory management
#define PAGE_SIZE 4096
#define HEAP_BLOCKS 1024

typedef struct
{
  uint64_t size;
  uint8_t is_free;
} memory_block_t;

static memory_block_t *heap_blocks;
static void *heap_start;

// Initialize memory management system
void
init_heap ()
{
  heap_start = (void *)0x100000;
  heap_blocks = (memory_block_t *)heap_start;
  heap_blocks[0].size = HEAP_BLOCKS * PAGE_SIZE;
  heap_blocks[0].is_free = 1;
}

// Simple memory allocator
void *
kmalloc (size_t size)
{
  for (int i = 0; i < HEAP_BLOCKS; i++)
    {
      if (heap_blocks[i].is_free && heap_blocks[i].size >= size)
        {
          if (heap_blocks[i].size > size + sizeof (memory_block_t))
            {
              uint64_t remaining
                  = heap_blocks[i].size - size - sizeof (memory_block_t);
              heap_blocks[i].size = size;

              memory_block_t *next
                  = (memory_block_t *)((char *)&heap_blocks[i]
                                       + sizeof (memory_block_t) + size);
              next->size = remaining;
              next->is_free = 1;
            }

          heap_blocks[i].is_free = 0;
          return (void *)((char *)&heap_blocks[i] + sizeof (memory_block_t));
        }
    }
  return NULL;
}

// Process management
void
create_process (void (*start_routine) (void))
{
  if (next_pid >= MAX_PROCESSES)
    return;

  PCB *process = &process_table[next_pid];
  process->pid = next_pid;
  process->state = PROCESS_READY;
  process->time_slice = 100;

  process->stack = kmalloc (STACK_SIZE);
  if (!process->stack)
    return;

  uint64_t *stack_ptr = (uint64_t *)((char *)process->stack + STACK_SIZE);
  *(--stack_ptr) = (uint64_t)start_routine;
  *(--stack_ptr) = 0;

  process->rsp = (uint64_t)stack_ptr;
  process->rbp = (uint64_t)stack_ptr;

  next_pid++;
}

// Simple scheduler
void
schedule ()
{
  uint64_t next_process = (current_pid + 1) % next_pid;

  while (next_process != current_pid)
    {
      if (process_table[next_process].state == PROCESS_READY)
        {
          PCB *old = &process_table[current_pid];
          PCB *new = &process_table[next_process];

          asm volatile ("mov %%rsp, %0" : "=r"(old->rsp));
          asm volatile ("mov %%rbp, %0" : "=r"(old->rbp));

          asm volatile ("mov %0, %%rsp" ::"r"(new->rsp));
          asm volatile ("mov %0, %%rbp" ::"r"(new->rbp));

          current_pid = next_process;
          return;
        }
      next_process = (next_process + 1) % next_pid;
    }
}

// System call handler
void
handle_syscall (uint64_t syscall_number, uint64_t param1, uint64_t param2)
{
  switch (syscall_number)
    {
    case 1: // write
      break;
    case 2: // read
      break;
    case 3: // exit
      process_table[current_pid].state = PROCESS_BLOCKED;
      schedule ();
      break;
    }
}

// Kernel's process scheduler update function
// This will be called from the assembly timer handler
void
kernel_timer_update ()
{
  static uint64_t ticks = 0;
  ticks++;

  if (ticks % process_table[current_pid].time_slice == 0)
    {
      schedule ();
    }
}

// Initial process that runs after kernel initialization
void
init_process ()
{
  while (1)
    {
      asm volatile ("hlt");
    }
}

void
kernel_main ()
{
  init_gdt ();
  init_idt ();
  init_memory ();
  init_io ();
  init_timer (50);
  init_keyboard ();
  init_disk ();
  init_heap ();

  // Create initial process
  create_process (init_process);

  // Enable interrupts
  asm volatile ("sti");

  while (1)
    {
      asm volatile ("hlt");
    }
}
