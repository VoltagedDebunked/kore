// SPDX-LICENSE-Identifier: GPL-3.0
/*
 * Copyright Rusin Danilo <rusindanilo@gmail.com> VoltagedDebunked
 */
#include "memory.h"
#include <stdbool.h>
#include <stddef.h>

// Memory configuration
#define MEMORY_SIZE 1024 * 1024 // 1 MB
#define BLOCK_SIZE 64           // 64 bytes
#define BLOCK_COUNT (MEMORY_SIZE / BLOCK_SIZE)
#define MAX_ALLOCATION_RETRIES 3
#define ALIGNMENT 8         // 8-byte alignment
#define MEMORY_PATTERN 0xAA // Pattern to fill freed memory

// Memory block metadata
typedef struct
{
  size_t size;       // Size of the block
  bool is_free;      // Block status
  void *next;        // Next block in the list
  uint32_t magic;    // Magic number for validation
  uint32_t checksum; // Simple checksum for corruption detection
} BlockHeader;

#define HEADER_SIZE sizeof (BlockHeader)
#define MAGIC_NUMBER 0xDEADBEEF

// Memory pool and management structures
static char memory_pool[MEMORY_SIZE] __attribute__ ((aligned (ALIGNMENT)));
static char *free_blocks[BLOCK_COUNT];
static int free_block_count = 0;
static BlockHeader *first_block = NULL;
static size_t total_allocated = 0;
static size_t peak_memory_usage = 0;
static size_t allocation_count = 0;
static size_t free_count = 0;

// Forward declarations
static BlockHeader *find_best_fit (size_t size);
static void coalesce_free_blocks (void);
static uint32_t calculate_checksum (BlockHeader *header);
static bool validate_block (BlockHeader *header);
static void *align_pointer (void *ptr, size_t alignment);

// Kernel-level memory operations
static void
k_memset (void *dest, uint8_t val, size_t len)
{
  uint8_t *dst = (uint8_t *)dest;
  for (size_t i = 0; i < len; i++)
    {
      dst[i] = val;
    }
}

static void
k_memcpy (void *dest, const void *src, size_t len)
{
  const uint8_t *s = (const uint8_t *)src;
  uint8_t *d = (uint8_t *)dest;
  for (size_t i = 0; i < len; i++)
    {
      d[i] = s[i];
    }
}

// Initialize memory pool with advanced features
void
init_memory ()
{
  k_memset (memory_pool, 0, MEMORY_SIZE);

  // Initialize first block
  first_block = (BlockHeader *)memory_pool;
  first_block->size = MEMORY_SIZE - HEADER_SIZE;
  first_block->is_free = true;
  first_block->next = NULL;
  first_block->magic = MAGIC_NUMBER;
  first_block->checksum = calculate_checksum (first_block);

  // Initialize free blocks array
  for (int i = 0; i < BLOCK_COUNT; i++)
    {
      free_blocks[i] = &memory_pool[i * BLOCK_SIZE];
    }
  free_block_count = BLOCK_COUNT;

  // Reset statistics
  total_allocated = 0;
  peak_memory_usage = 0;
  allocation_count = 0;
  free_count = 0;
}

// Calculate checksum for memory block
static uint32_t
calculate_checksum (BlockHeader *header)
{
  uint32_t sum = 0;
  uint8_t *ptr = (uint8_t *)header;

  // Skip checksum field in calculation
  for (size_t i = 0; i < offsetof (BlockHeader, checksum); i++)
    {
      sum += ptr[i];
    }
  return sum;
}

// Validate memory block integrity
static bool
validate_block (BlockHeader *header)
{
  if (!header)
    return false;
  if (header->magic != MAGIC_NUMBER)
    return false;
  if (calculate_checksum (header) != header->checksum)
    return false;
  return true;
}

// Align pointer to specified alignment
static void *
align_pointer (void *ptr, size_t alignment)
{
  uintptr_t addr = (uintptr_t)ptr;
  uintptr_t aligned = (addr + (alignment - 1)) & ~(alignment - 1);
  return (void *)aligned;
}

// Find best fitting block using best-fit algorithm
static BlockHeader *
find_best_fit (size_t size)
{
  BlockHeader *current = first_block;
  BlockHeader *best_fit = NULL;
  size_t smallest_diff = MEMORY_SIZE;

  while (current)
    {
      if (!validate_block (current))
        {
          return NULL;
        }

      if (current->is_free && current->size >= size)
        {
          size_t diff = current->size - size;
          if (diff < smallest_diff)
            {
              smallest_diff = diff;
              best_fit = current;
            }
        }
      current = current->next;
    }
  return best_fit;
}

// Coalesce adjacent free blocks
static void
coalesce_free_blocks (void)
{
  BlockHeader *current = first_block;

  while (current && current->next)
    {
      if (!validate_block (current) || !validate_block (current->next))
        {
          return;
        }

      if (current->is_free && ((BlockHeader *)current->next)->is_free)
        {
          current->size += HEADER_SIZE + ((BlockHeader *)current->next)->size;
          current->next = ((BlockHeader *)current->next)->next;
          current->checksum = calculate_checksum (current);
        }
      else
        {
          current = current->next;
        }
    }
}

// Enhanced memory allocation with retry mechanism
void *
allocate_memory (int size)
{
  if (size <= 0)
    return NULL;

  size_t aligned_size = (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
  BlockHeader *block = NULL;
  int retries = 0;

  while (retries < MAX_ALLOCATION_RETRIES)
    {
      block = find_best_fit (aligned_size);

      if (block)
        {
          if (block->size >= aligned_size + HEADER_SIZE + BLOCK_SIZE)
            {
              // Split block if remaining size is sufficient
              BlockHeader *new_block
                  = (BlockHeader *)((char *)block + HEADER_SIZE
                                    + aligned_size);
              new_block->size = block->size - aligned_size - HEADER_SIZE;
              new_block->is_free = true;
              new_block->next = block->next;
              new_block->magic = MAGIC_NUMBER;
              new_block->checksum = calculate_checksum (new_block);

              block->size = aligned_size;
              block->next = new_block;
            }

          block->is_free = false;
          block->checksum = calculate_checksum (block);

          // Update statistics
          total_allocated += block->size;
          allocation_count++;
          peak_memory_usage = (total_allocated > peak_memory_usage)
                                  ? total_allocated
                                  : peak_memory_usage;

          return (void *)((char *)block + HEADER_SIZE);
        }

      // Try to coalesce and retry
      coalesce_free_blocks ();
      retries++;
    }

  return NULL;
}

// Enhanced memory deallocation with security features (yes i know this sounds stupid)
void
free_memory (void *ptr)
{
  if (!ptr)
    return;

  BlockHeader *header = (BlockHeader *)((char *)ptr - HEADER_SIZE);

  if (!validate_block (header))
    {
      return;
    }

  if (header->is_free)
    {
      // Double free detection
      return;
    }

  // Clear memory contents
  k_memset (ptr, MEMORY_PATTERN, header->size);

  header->is_free = true;
  header->checksum = calculate_checksum (header);

  // Update statistics
  total_allocated -= header->size;
  free_count++;

  coalesce_free_blocks ();
}

// Get memory statistics
void
get_memory_stats (MemoryStats *stats)
{
  if (!stats)
    return;

  stats->total_memory = MEMORY_SIZE;
  stats->used_memory = total_allocated;
  stats->free_memory = MEMORY_SIZE - total_allocated;
  stats->allocation_count = allocation_count;
  stats->free_count = free_count;
  stats->peak_usage = peak_memory_usage;

  // Calculate fragmentation
  size_t largest_free_block = 0;
  BlockHeader *current = first_block;
  while (current)
    {
      if (current->is_free && current->size > largest_free_block)
        {
          largest_free_block = current->size;
        }
      current = current->next;
    }
  stats->fragmentation
      = (stats->free_memory > 0)
            ? (1.0 - (double)largest_free_block / stats->free_memory) * 100
            : 0;
}

// Memory pool debugging function
void
debug_memory_pool (void)
{
  BlockHeader *current = first_block;
  size_t block_count = 0;

  while (current)
    {
      if (!validate_block (current))
        {
          return;
        }
      current = current->next;
      block_count++;
    }
}

// Reallocate memory block
void *
reallocate_memory (void *ptr, size_t new_size)
{
  if (!ptr)
    return allocate_memory (new_size);
  if (new_size == 0)
    {
      free_memory (ptr);
      return NULL;
    }

  BlockHeader *header = (BlockHeader *)((char *)ptr - HEADER_SIZE);
  if (!validate_block (header))
    return NULL;

  if (header->size >= new_size)
    {
      // Current block is large enough
      return ptr;
    }

  // Allocate new block and copy data
  void *new_ptr = allocate_memory (new_size);
  if (!new_ptr)
    return NULL;

  k_memcpy (new_ptr, ptr, header->size);
  free_memory (ptr);

  return new_ptr;
}
