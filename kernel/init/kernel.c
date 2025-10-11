#include <stdio.h>
#include <stddef.h>
#include <assert.h>

#include <kernel/tty.h>
#include <kernel/multiboot.h>
#include <kernel/debug.h>
#include <kernel/gdt.h>
#include <kernel/interrupts.h>
#include <kernel/keyboard.h>
#include <kernel/paging.h>
#include <kernel/kheap.h>
#include <kernel/shell.h>

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
    printf("\n");

    init_shell();
    /* The code below is unreachable because init_shell() never returns.
     * It's kept here as a fallback idle loop in case the shell ever exits. */
    while (1) {
        asm volatile("hlt");
    }
}
