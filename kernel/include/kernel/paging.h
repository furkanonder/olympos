#ifndef KERNEL_PAGING_H
#define KERNEL_PAGING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * Memory layout constants
 * 
 * We support up to 128 MiB of physical memory, divided into 4 KiB frames.
 * The kernel is identity-mapped and reserves the first 8 MiB for its use.
 */
#define PAGE_SIZE           4096                                /* 4 KiB pages */
#define FRAME_SIZE          PAGE_SIZE
#define NUM_FRAMES          (128 * 1024 * 1024 / FRAME_SIZE)    /* 32768 frames for 128 MiB */
#define KMEM_MAX            (8 * 1024 * 1024)                   /* 8 MiB reserved for kernel */

/**
 * Page Directory and Page Table entry flags
 */
#define PDE_PRESENT         0x1     /* Page is present in memory */
#define PDE_WRITABLE        0x2     /* Page is writable */

#define PTE_PRESENT         PDE_PRESENT
#define PTE_WRITABLE        PDE_WRITABLE

/**
 * Page Directory Entry (PDE)
 * 
 * 31                   12 11  9 8  7 6 5  4   3   2   1  0
 * +----------------------+-----+-+--+-+-+---+---+---+---+-+
 * |  Page Table Address  |Avail|G|PS|0|A|PCD|PWT|U/S|R/W|P|
 * +----------------------+-----+-+--+-+-+---+---+---+---+-+
 *
 * Bit 0    (P)   - Present: Page table is present in memory
 * Bit 1    (R/W) - Read/Write: 0 = read-only, 1 = read/write
 * Bit 2    (U/S) - User/Supervisor: 0 = supervisor, 1 = user
 * Bit 3    (PWT) - Page-level Write-Through
 * Bit 4    (PCD) - Page-level Cache Disable
 * Bit 5    (A)   - Accessed: Set by CPU when page table is accessed
 * Bit 6    (0)   - Reserved (must be 0)
 * Bit 7    (PS)  - Page Size: 0 = 4KB pages, 1 = 4MB pages
 * Bit 8    (G)   - Global (Ignored for PDEs)
 * Bits 9-11      - Available for OS use
 * Bits 12-31     - Page Table 4KB-aligned physical address
 */
typedef uint32_t pde_t;

/**
 * Page Table Entry (PTE)
 * 
 * 31                 12 11  9 8  7  6 5  4   3   2   1  0
 * +--------------------+-----+-+---+-+-+---+---+---+---+-+
 * | Physical Page Addr |Avail|G|PAT|D|A|PCD|PWT|U/S|R/W|P|
 * +--------------------+-----+-+---+-+-+---+---+---+---+-+
 * 
 * Bit 0    (P)   - Present: Page is present in memory
 * Bit 1    (R/W) - Read/Write: 0 = read-only, 1 = read/write
 * Bit 2    (U/S) - User/Supervisor: 0 = supervisor, 1 = user
 * Bit 3    (PWT) - Page-level Write-Through
 * Bit 4    (PCD) - Page-level Cache Disable
 * Bit 5    (A)   - Accessed: Set by CPU when page is accessed
 * Bit 6    (D)   - Dirty: Set by CPU when page is written to
 * Bit 7    (PAT) - Page Attribute Table (if PAT supported)
 * Bit 8    (G)   - Global: Page isn't flushed from TLB on CR3 reload
 * Bits 9-11      - Available for OS use
 * Bits 12-31     - Physical page frame 4KB-aligned address
 */
typedef uint32_t pte_t;

/**
 * Page Directory - contains 1024 PDEs
 */
typedef struct {
    pde_t entries[1024];
} __attribute__((aligned(PAGE_SIZE))) page_directory_t;

/**
 * Page Table - contains 1024 PTEs
 */
typedef struct {
    pte_t entries[1024];
} __attribute__((aligned(PAGE_SIZE))) page_table_t;

/**
 * Initialize the paging system and switch to paging mode
 */
void paging_init(void);

/**
 * Allocate a physical frame
 * 
 * @return Physical address of the allocated frame, or 0 if none available
 */
uint32_t frame_alloc(void);

/**
 * Free a physical frame
 * 
 * @param frame_addr Physical address of the frame to free
 */
void frame_free(uint32_t frame_addr);

#endif
