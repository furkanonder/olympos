#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include <kernel/paging.h>
#include <kernel/interrupts.h>
#include <kernel/debug.h>

/**
 * Ultra-Simple Page Frame Allocator & Paging
 * 
 * Based on: https://wiki.osdev.org/Writing_A_Page_Frame_Allocator
 * 
 * Once paging is enabled, the CPU's MMU (Memory Management Unit) ALWAYS translates virtual
 * addresses to physical addresses using page tables.
 * 
 * Virtual Address (32-bit) breakdown:
 *   Bits 31-22 (10 bits): Page Directory Index (1024 entries)
 *   Bits 21-12 (10 bits): Page Table Index (1024 entries per table)
 *   Bits 11-0  (12 bits): Offset within page (4096 bytes)
 * 
 * Translation process (automatic by CPU):
 *   1. CR3 register points to Page Directory
 *   2. Use PD index to find Page Table
 *   3. Use PT index to find Physical Frame address
 *   4. Add offset to get final physical address
 * 
 * Our kernel uses IDENTITY MAPPING (virtual = physical) for simplicity:
 *   Virtual 0x100000 → Physical 0x100000
 *   Virtual 0x500000 → Physical 0x500000
 * 
 * Implementation details:
 * - Statically allocate page directory and tables (no dynamic allocation)
 * - Bitmap for frame allocation (first - fit algorithm)
 * - Identity map kernel region (0x0 - KMEM_MAX = 8MB)
 * - Unmapped addresses trigger page faults (ISR #14)
 */

/* External variable from debug.c marking where kernel sections end */
extern uint32_t elf_sections_end;

/* Page directory (1024 entries = 4KB) - statically allocated */
static page_directory_t kernel_page_directory __attribute__((aligned(4096)));

/* Page tables for kernel identity mapping
 * We need 2 tables to map 8MB (0x0 - 0x800000)
 * Each table maps 4MB (1024 pages * 4KB) */
static page_table_t kernel_page_tables[2] __attribute__((aligned(4096)));

/* Frame bitmap: 1 bit per 4KB frame (0=free, 1=used) */
static uint32_t frame_bitmap[NUM_FRAMES / 32];

/**
 * Bitmap helper functions (inline for performance)
 */
static inline bool frame_test(uint32_t frame_num) {
    uint32_t idx = frame_num / 32;
    uint32_t bit = frame_num % 32;
    return (frame_bitmap[idx] & (1 << bit)) != 0;
}

static inline void frame_set(uint32_t frame_num) {
    uint32_t idx = frame_num / 32;
    uint32_t bit = frame_num % 32;
    frame_bitmap[idx] |= (1 << bit);
}

static inline void frame_clear(uint32_t frame_num) {
    uint32_t idx = frame_num / 32;
    uint32_t bit = frame_num % 32;
    frame_bitmap[idx] &= ~(1 << bit);
}

/**
 * Initialize frame bitmap
 * Mark kernel binary frames as used, leave rest free for allocation
 */
static void frame_bitmap_init(void) {
    /* Mark all frames as free initially */
    memset(frame_bitmap, 0, sizeof(frame_bitmap));
    /* Calculate how many frames the kernel binary occupies */
    uint32_t kernel_frames = (elf_sections_end + FRAME_SIZE - 1) / FRAME_SIZE;
    /* Mark only the kernel binary frames as allocated */
    for (uint32_t i = 0; i < kernel_frames; i++) {
        frame_set(i);
    }
}

/**
 * Allocate a physical frame
 * Returns physical address of frame, or 0 if none available
 */
uint32_t frame_alloc(void) {
    /* Simple first-fit: scan bitmap for first free frame */
    for (uint32_t i = 0; i < NUM_FRAMES; i++) {
        if (!frame_test(i)) {
            frame_set(i);
            return i * FRAME_SIZE;
        }
    }
    /* Out of memory */
    return 0;
}

/**
 * Free a physical frame
 */
void frame_free(uint32_t frame_addr) {
    uint32_t frame_num = frame_addr / FRAME_SIZE;
    if (frame_num >= NUM_FRAMES) {
        printf("frame_free: Invalid frame address %p\n", frame_addr);
        return;
    }
    frame_clear(frame_num);
}

/**
 * Map a virtual page to a physical frame
 * 
 * This function sets up the page tables so the MMU can translate
 * the given virtual address to the specified physical address.
 * 
 * Example: map_page(pd, 0x100000, 0x100000, PRESENT | WRITABLE)
 *   Virtual address 0x100000 breakdown:
 *     Bits 31-22: 0x0 (PD index 0)
 *     Bits 21-12: 0x100 (PT index 256)
 *     Bits 11-0:  0x000 (offset within page)
 * 
 * @param page_dir Page directory
 * @param virt_addr Virtual address to map
 * @param phys_addr Physical address to map to
 * @param flags Page flags (present, writable, etc.)
 */
static void map_page(page_directory_t* page_dir, uint32_t virt_addr, uint32_t phys_addr, uint32_t flags) {
    /* Extract indices from virtual address using bit manipulation */
    uint32_t pd_idx = virt_addr >> 22;           /* Top 10 bits: Page Directory index */
    uint32_t pt_idx = (virt_addr >> 12) & 0x3FF; /* Middle 10 bits: Page Table index (0x3FF = 1023) */
    /* Get page directory entry */
    pde_t* pde = &page_dir->entries[pd_idx];
    
    /* Get page table address from PDE
     * 
     * Why ~0xFFF? Page table entries have this format:
     * 
     *   31                               12 11        0
     *   +----------------------------------+-----------+
     *   |  Physical Address (20 bits)      | Flags     |
     *   +----------------------------------+-----------+
     * 
     * - Upper 20 bits (31-12): Physical frame address (4KB aligned)
     * - Lower 12 bits (11-0):  Flags (PRESENT, WRITABLE, USER, etc.)
     * 
     * 0xFFF = 0000 1111 1111 1111 (lower 12 bits set)
     * ~0xFFF = 1111 0000 0000 0000 (upper 20 bits set, lower 12 clear)
     * 
     * Using & ~0xFFF masks off the flag bits to get just the address.
     */
    page_table_t* page_table = (page_table_t*) (*pde & ~0xFFF);
    if (!page_table) {
        printf("[FAILED] map_page: Page table not allocated for index %u!\n", pd_idx);
        return;
    }
    
    /* Set the page table entry: physical frame address + flags
     * (phys_addr & ~0xFFF) clears lower 12 bits (ensures 4KB alignment)
     * | flags combines the address with flag bits */
    page_table->entries[pt_idx] = (phys_addr & ~0xFFF) | flags;
}

/**
 * Create identity mapping for kernel
 * 
 * Identity mapping means: virtual address = physical address
 * This allows kernel code to run without address translation complexity.
 * 
 * We map 8MB (KMEM_MAX) of memory:
 *   Virtual 0x000000 → Physical 0x000000
 *   Virtual 0x100000 → Physical 0x100000
 *   ...
 *   Virtual 0x7FFFFF → Physical 0x7FFFFF
 * 
 * After this function, the page tables are set up but translation is NOT
 * active yet. enable_paging() must be called to activate the MMU.
 */
static void setup_identity_mapping(void) {
    /* Clear page directory */
    memset(&kernel_page_directory, 0, sizeof(page_directory_t));
    /* Clear page tables */
    memset(kernel_page_tables, 0, sizeof(kernel_page_tables));
    
    /* Install page tables in page directory
     * 
     * Each page table has 1024 entries, each entry maps 4KB
     * One page table covers: 1024 * 4KB = 4MB
     * 
     * We want to map 8MB (KMEM_MAX), so we need: 8MB / 4MB = 2 page tables
     * 
     * Memory layout:
     *   PD[0] → page_table[0] covers virtual 0x000000 - 0x3FFFFF (0 - 4MB)
     *   PD[1] → page_table[1] covers virtual 0x400000 - 0x7FFFFF (4 - 8MB)
     */
    kernel_page_directory.entries[0] = ((uint32_t) &kernel_page_tables[0]) | PDE_PRESENT | PDE_WRITABLE;
    kernel_page_directory.entries[1] = ((uint32_t) &kernel_page_tables[1]) | PDE_PRESENT | PDE_WRITABLE;
    
    /* Identity map the kernel region (virtual = physical) for all 2048 pages
     * 2048 pages = 8MB / 4KB = 8192KB / 4KB */
    for (uint32_t addr = 0; addr < KMEM_MAX; addr += PAGE_SIZE) {
        map_page(&kernel_page_directory, addr, addr, PTE_PRESENT | PTE_WRITABLE);
    }
}

/**
 * Enable paging by loading page directory and setting CR0
 * 
 * Steps:
 *   1. Load CR3 with page directory address (MMU uses this as translation root)
 *   2. Set PG bit (bit 31) in CR0 register to activate paging
 * 
 * From this point on, every memory access follows this process:
 *   CPU uses virtual address → MMU consults page tables → Physical address accessed
 * 
 * Unmapped virtual addresses will trigger a page fault (ISR #14).
 */
static void enable_paging(page_directory_t* page_dir) {
    /* Load page directory address into CR3 (tells MMU where page tables are) */
    asm volatile("mov %0, %%cr3" :: "r"(page_dir));
    /* Enable paging by setting PG bit (bit 31) in CR0 */
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;  /* Set PG bit - translation now active! */
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}

/**
 * Page fault handler (ISR #14)
 * Called when CPU tries to access unmapped/invalid memory
 */
static void page_fault_handler(regs_t* regs) {
    /* CR2 holds the faulting address */
    uint32_t faulty_addr;
    asm volatile("mov %%cr2, %0" : "=r"(faulty_addr));

    printf("\n========================================\n");
    printf("PAGE FAULT!\n");
    printf("========================================\n");
    printf("Faulty address:  %p\n", faulty_addr);
    printf("Present:         %s\n", regs->err_code & 0x1 ? "yes" : "no");
    printf("Operation:       %s\n", regs->err_code & 0x2 ? "write" : "read");
    printf("Mode:            %s\n", regs->err_code & 0x4 ? "user" : "kernel");
    printf("EIP:             %p\n", regs->eip);
    printf("Error code:      0x%x\n", regs->err_code);
    printf("========================================\n");

    panic("\nPage fault not handled - system halted.\n");
}

/**
 * Initialize paging system
 * 
 * Steps:
 * 1. Initialize frame bitmap
 * 2. Set up identity mapping (using static page tables)
 * 3. Register page fault handler
 * 4. Enable paging
 */
void paging_init(void) {
    /* Step 1: Initialize frame allocator */
    frame_bitmap_init();
    /* Step 2: Set up identity mapping using static structures */
    setup_identity_mapping();
    /* Step 3: Register page fault handler */
    register_isr(14, page_fault_handler);
    /* Step 4: Enable paging */
    enable_paging(&kernel_page_directory);
    printf("[  OK  ] Paging initialized successfully.\n");
}
