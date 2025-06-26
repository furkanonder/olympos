#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <stdbool.h>

#include <kernel/tty.h>
#include <kernel/multiboot.h>
#include <kernel/debug.h>
#include <kernel/serial.h>
#include <kernel/gdt.h>

/**
 * Kernel entry point
 */
void kernel_main(unsigned long magic, unsigned long addr) {
    assert(magic == MULTIBOOT_BOOTLOADER_MAGIC && "Invalid bootloader magic");
    multiboot_info_t* mbi = (multiboot_info_t*) addr;
    debug_initialize(mbi);

    terminal_initialize();
    gdt_init();

    printf("=======================================\n");
    printf("Welcome to Olympos\n");
    printf("An experimental 32-bit Operating System\n");
    printf("=======================================\n");
}
