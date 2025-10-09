#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include <kernel/kheap.h>
#include <kernel/paging.h>

/**
 * Simple Bitmap-Based Heap Allocator
 * Based on: https://wiki.osdev.org/User:Pancakes/BitmapHeapImplementation
 */

#define HEAP_BLOCK_SIZE     4096                     /* 4 KB blocks */
#define HEAP_BLOCKS_MAX     2048                     /* 8 MB total heap (2048 * 4KB) */
#define BITMAP_SIZE         (HEAP_BLOCKS_MAX / 8)    /* 256 bytes for bitmap */

/* External from debug.c - marks where kernel sections end */
extern uint32_t elf_sections_end;

/* Start of the heap */
uint32_t kheap_curr = 0;

/* Bitmap: 1 bit per block (0=free, 1=used) */
static uint8_t heap_bitmap[BITMAP_SIZE];

/* Heap start address */
static uint32_t heap_start = 0;

/* Number of blocks allocated */
static uint32_t blocks_used = 0;

/**
 * Check if a block is allocated
 */
static inline bool is_block_used(uint32_t block_idx) {
    uint32_t byte_idx = block_idx / 8;
    uint32_t bit_idx = block_idx % 8;
    return (heap_bitmap[byte_idx] & (1 << bit_idx)) != 0;
}

/**
 * Mark a block as used
 */
static inline void mark_block_used(uint32_t block_idx) {
    uint32_t byte_idx = block_idx / 8;
    uint32_t bit_idx = block_idx % 8;
    heap_bitmap[byte_idx] |= (1 << bit_idx);
}

/**
 * Mark a block as free
 */
static inline void mark_block_free(uint32_t block_idx) {
    uint32_t byte_idx = block_idx / 8;
    uint32_t bit_idx = block_idx % 8;
    heap_bitmap[byte_idx] &= ~(1 << bit_idx);
}

/**
 * Find N contiguous free blocks (First-Fit with Skip-Ahead)
 *
 * Algorithm:
 *   1. Start at block i
 *   2. Check blocks i, i + 1, i + 2, ... i + (count - 1) for contiguous free space
 *   3. If block at i + j is used, skip ahead to i + j + 1 (optimization)
 *   4. Return starting index if found, -1 otherwise
 *
 * Example: Need 3 contiguous blocks
 *   Bitmap: [F F U F F F U ...]  (F=free, U=used)
 *   Index:   0 1 2 3 4 5 6
 *
 *   i=0: Check [0,1,2] → Block 2 used (j=2) → Skip to i=2
 *   i=2: Start at used block → Skip immediately
 *   i=3: Check [3,4,5] → All free! Return 3
 *
 * @param count Number of contiguous blocks needed
 * @return Starting block index, or -1 if not found
 */
static int32_t find_free_blocks(uint32_t count) {
    for (uint32_t i = 0; i < HEAP_BLOCKS_MAX; i++) {
        /* Assume we'll find enough contiguous blocks starting here */
        bool all_free = true;
        /* Check 'count' blocks starting from position i */
        for (uint32_t j = 0; j < count && (i + j) < HEAP_BLOCKS_MAX; j++) {
            if (is_block_used(i + j)) {
                /* Found a used block at position i + j
                 *
                 * Skip-ahead optimization: Since blocks i, i + 1, ... i + j - 1 are free
                 * but i + j is used, we know none of those can be valid starting points.
                 * Jump directly to i + j (the outer loop will increment to i + j + 1).
                 *
                 * Example: i = 0, found used block at j = 2
                 *   [F F U ...] → Skip i = 0,1,2 directly to i = 2
                 *         ^
                 *       i + j = 2
                 */
                all_free = false;
                i += j;  /* Skip ahead */
                break;
            }
        }
        /* Check if we found enough contiguous blocks and they fit within bounds */
        if (all_free && (i + count) <= HEAP_BLOCKS_MAX) {
            return i;  /* Success: found starting block index */
        }
    }
    /* No contiguous blocks found - return -1 to indicate failure */
    return -1;
}

/**
 * Initialize the bitmap heap allocator
 */
void kheap_init(void) {
    /* Heap starts right after kernel sections end */
    kheap_curr = elf_sections_end;
    /* Align to block boundary (4KB) - rounds UP to next 4096-byte boundary
     * Formula: (addr + alignment - 1) & ~(alignment - 1)
     * Example: If elf_sections_end = 0x106789
     *   Step 1: 0x106789 + 0xFFF = 0x107788
     *   Step 2: 0x107788 & 0xFFFFF000 = 0x107000 (aligned)
     */
    kheap_curr = (kheap_curr + HEAP_BLOCK_SIZE - 1) & ~(HEAP_BLOCK_SIZE - 1);
    heap_start = kheap_curr;
    /* Clear the bitmap (all blocks free) */
    memset(heap_bitmap, 0, BITMAP_SIZE);
    blocks_used = 0;
    printf("[  OK  ] Heap initialized at %p\n", heap_start);
}

/**
 * Allocate memory from kernel heap
 *
 * Memory Layout:
 * ┌───────────────┬──────────────────┬──────────────┐
 * │ Metadata (4B) │    User Data     │ Unused Space │
 * │ [block count] │ (requested size) │    (waste)   │
 * └───────────────┴──────────────────┴──────────────┘
 *  ↑               ↑
 *  ptr             ptr + 1 (returned to user)
 *
 * The metadata (block count) is stored BEFORE the user data.
 * We return (ptr + 1) which skips over the metadata, so the user
 * never sees or modifies it. On free, we go back (ptr - 1) to find it.
 *
 * Example: kmalloc(8) with heap_start = 0x11E000, block 0 allocated:
 *   0x11E000: [0x00000001]  ← Metadata (1 block used) - HIDDEN
 *   0x11E004: [User's 8 bytes of data starts here] ← RETURNED to user
 *   0x11E00C: [Unused 4084 bytes...]
 *
 * Strategy:
 * 1. Calculate blocks needed (data + metadata)
 * 2. Find contiguous free blocks
 * 3. Store block count at block start
 * 4. Mark blocks as used
 * 5. Return pointer skipping metadata (ptr + 1)
 */
void* kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    /* Calculate blocks needed: requested size + 4 bytes for metadata */
    size_t total_size = size + sizeof(uint32_t);
    uint32_t blocks_needed = (total_size + HEAP_BLOCK_SIZE - 1) / HEAP_BLOCK_SIZE;
    if (blocks_needed > HEAP_BLOCKS_MAX) {
        printf("[FAILED] kmalloc: Request too large (%zu bytes, %u blocks)\n", size, blocks_needed);
        return NULL;
    }
    /* Find free blocks using first-fit */
    int32_t start_block = find_free_blocks(blocks_needed);
    if (start_block < 0) {
        printf("[FAILED] kmalloc: Out of memory! (need %u blocks for %zu bytes)\n", blocks_needed, size);
        return NULL;
    }
    /* Mark blocks as used in bitmap */
    for (uint32_t i = 0; i < blocks_needed; i++) {
        mark_block_used(start_block + i);
    }
    blocks_used += blocks_needed;
    /* Get pointer to start of allocated blocks */
    uint32_t* ptr = (uint32_t*) (heap_start + (start_block * HEAP_BLOCK_SIZE));
    /* Store metadata (block count) at the very beginning. This occupies the first 4 bytes of the allocation */
    *ptr = blocks_needed;
    /* Return pointer AFTER metadata (skip 4 bytes)
     * ptr + 1 moves forward by sizeof(uint32_t) = 4 bytes. User receives clean pointer to usable space */
    return (void*) (ptr + 1);
}

/**
 * Free previously allocated memory
 *
 * Metadata Recovery:
 * User has pointer to data (e.g., 0x11E004)
 * We need to go BACK 4 bytes to find metadata (0x11E000)
 *
 * Memory Layout:
 * ┌───────────────┬───────────┐
 * │ Metadata (4B) │ User Data │
 * └───────────────┴───────────┘
 *  ↑               ↑
 *  ptr - 1         ptr (what user gave us)
 *
 * Example: User calls kfree(0x11E004)
 *   count_ptr = 0x11E004 - 1 = 0x11E000  ← Go back to metadata
 *   blocks_to_free = *0x11E000 = 1       ← Read block count
 *   Free block 0                         ← Mark as free in bitmap
 *
 * Strategy:
 * 1. Subtract 1 from user pointer to get metadata pointer
 * 2. Read block count from metadata
 * 3. Calculate block index from address
 * 4. Mark all blocks as free in bitmap
 */
void kfree(void* ptr) {
    if (ptr == NULL) {
        return;
    }
    /* Get metadata pointer by going BACK 4 bytes. (ptr - 1) moves back by sizeof(uint32_t) = 4 bytes */
    uint32_t* count_ptr = ((uint32_t*) ptr) - 1;
    /* Read block count from metadata */
    uint32_t blocks_to_free = *count_ptr;
    /* Calculate which block this is (using metadata address, not user pointer!)
     * Cast pointer to uint32_t to get numerical address value */
    uint32_t block_addr = (uint32_t) count_ptr;
    if (block_addr < heap_start) {
        printf("kfree: Invalid pointer %p (before heap start)\n", ptr);
        return;
    }
    /* Calculate block index: offset from heap start divided by block size */
    uint32_t start_block = (block_addr - heap_start) / HEAP_BLOCK_SIZE;
    if (start_block >= HEAP_BLOCKS_MAX) {
        printf("[FAILED] kfree: Invalid pointer %p (beyond heap)\n", ptr);
        return;
    }
    /* Sanity check */
    if (blocks_to_free == 0 || blocks_to_free > HEAP_BLOCKS_MAX) {
        printf("[FAILED] kfree: Corrupted block count %u at %p\n", blocks_to_free, ptr);
        return;
    }
    /* Free all blocks */
    for (uint32_t i = 0; i < blocks_to_free; i++) {
        mark_block_free(start_block + i);
    }
    blocks_used -= blocks_to_free;
}

/**
 * Get heap statistics (for debugging)
 */
void kheap_stats(void) {
    uint32_t free_blocks = HEAP_BLOCKS_MAX - blocks_used;
    printf("Heap statistics:\n");
    printf("Blocks used:  %u / %u\n", blocks_used, HEAP_BLOCKS_MAX);
    printf("Blocks free:  %u\n", free_blocks);
    printf("Memory used:  %u KB\n", (blocks_used * HEAP_BLOCK_SIZE) / 1024);
    printf("Memory free:  %u KB\n", (free_blocks * HEAP_BLOCK_SIZE) / 1024);
}
