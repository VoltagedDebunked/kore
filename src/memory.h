// SPDX-LICENSE-Identifier: GPL-3.0
/*
 * Copyright Rusin Danilo <rusindanilo@gmail.com> VoltagedDebunked
 */
#ifndef MEMORY_H
#define MEMORY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Memory statistics structure
 * Contains information about memory usage and allocation patterns
 */
typedef struct
{
  size_t total_memory;     // Total size of memory pool
  size_t used_memory;      // Currently used memory
  size_t free_memory;      // Available free memory
  size_t allocation_count; // Number of successful allocations
  size_t free_count;       // Number of successful frees
  size_t peak_usage;       // Peak memory usage recorded
  size_t fragmentation;    // Memory fragmentation percentage
} MemoryStats;

/**
 * Initialize the memory management system
 * Must be called before any other memory functions
 */
void init_memory (void);

/**
 * Allocate a block of memory
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory or NULL if allocation fails
 */
void *allocate_memory (int size);

/**
 * Free a previously allocated memory block
 * @param ptr Pointer to memory block to free
 */
void free_memory (void *ptr);

/**
 * Reallocate a memory block with a new size
 * @param ptr Pointer to existing memory block
 * @param new_size New size in bytes
 * @return Pointer to reallocated memory or NULL if reallocation fails
 */
void *reallocate_memory (void *ptr, size_t new_size);

/**
 * Get current memory statistics
 * @param stats Pointer to MemoryStats structure to fill
 */
void get_memory_stats (MemoryStats *stats);

/**
 * Print debug information about memory pool
 * Displays information about each memory block
 */
void debug_memory_pool (void);

#endif /* MEMORY_H */
