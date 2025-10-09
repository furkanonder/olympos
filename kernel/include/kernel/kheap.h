#ifndef KERNEL_KHEAP_H
#define KERNEL_KHEAP_H

#include <stdint.h>
#include <stddef.h>

/**
 * Simple Bitmap-Based Heap Allocator
 * Based on: https://wiki.osdev.org/User:Pancakes/BitmapHeapImplementation
 */

/**
 * Initialize the kernel heap allocator
 *
 * Must be called after paging_init().
 */
void kheap_init(void);

/**
 * Allocate memory from the kernel heap
 *
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL on failure
 */
void* kmalloc(size_t size);

/**
 * Free previously allocated memory
 *
 * @param ptr Pointer to memory to free (from kmalloc())
 */
void kfree(void* ptr);

/**
 * Print heap statistics (for debugging)
 */
void kheap_stats(void);

#endif /* KERNEL_KHEAP_H */
