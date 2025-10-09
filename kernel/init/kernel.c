#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#include <kernel/tty.h>
#include <kernel/multiboot.h>
#include <kernel/debug.h>
#include <kernel/gdt.h>
#include <kernel/interrupts.h>
#include <kernel/keyboard.h>
#include <kernel/paging.h>
#include <kernel/kheap.h>

/**
 * Kernel entry point
 */
void kernel_main(unsigned long magic, unsigned long addr) {
    assert(magic == MULTIBOOT_BOOTLOADER_MAGIC && "Invalid bootloader magic");

    terminal_initialize();
    multiboot_info_t* mbi = (multiboot_info_t*) addr;
    debug_initialize(mbi);

    gdt_init();
    idt_init();
    paging_init();
    kheap_init();
    keyboard_initialize();

    printf("=======================================\n");
    printf("Welcome to Olympos\n");
    printf("An experimental 32-bit Operating System\n");
    printf("=======================================\n");

    printf("Supported physical memory size: %d MiB\n", NUM_FRAMES * 4 / 1024);
    printf("Reserved memory for the kernel: %d MiB\n", KMEM_MAX / 1024 / 1024);

    /* Our heap uses 4 KB (4096-byte) blocks. Each kmalloc(8):
     *   - Requests: 8 bytes
     *   - Needs: 8 + 4 (metadata) = 12 bytes total
     *   - Allocates: 1 block = 4096 bytes (minimum granularity)
     * ============================================================  */
    printf("Testing heap allocator...\n");

    /* Allocation 1: Request 8 bytes
     * Block 0 allocated → 4 KB used
     * Heap state: [USED] [FREE] [FREE] ... */
    void* a = kmalloc(8);

    /* Allocation 2: Request 8 bytes
     * Block 1 allocated → 4 KB used
     * Heap state: [USED] [USED] [FREE] ... */
    void* b = kmalloc(8);

    /* Allocation 3: Request 8 bytes
     * Block 2 allocated → 4 KB used
     * Heap state: [USED] [USED] [USED] [FREE] ... */
    void* c = kmalloc(8);

    printf("a: %p\n", a);
    printf("b: %p\n", b);
    printf("c: %p\n", c);

    /* Deallocation 1: Free block 2
     * Heap state: [USED] [USED] [FREE] [FREE] ... */
    kfree(c);

    /* Deallocation 2: Free block 1
     * Heap state: [USED] [FREE] [FREE] [FREE] ... */
    kfree(b);

    /* Allocation 4: Request 8 bytes
     * Block 1 reused (first-fit finds it) → 4 KB used
     * Heap state: [USED] [USED] [FREE] [FREE] ...
     *
     * Note: d gets same address as b (0x11f004) because block 1 was freed and is now reused. */
    void* d = kmalloc(8);
    printf("d: %p\n", d);

    /* Final Statistics:
     * Blocks used: 2 (block 0 for 'a', block 1 for 'd')
     * Memory used: 2 blocks × 4 KB = 8 KB
     * Actual data: 2 × 8 bytes = 16 bytes. */
    kheap_stats();

    /* Enter idle loop. CPU halts until woken by interrupts (keyboard, timer, etc.). */
    while (1) {
        asm volatile("hlt");
    }
}
