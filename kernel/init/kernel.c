#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <stdbool.h>

#include <kernel/tty.h>
#include <kernel/multiboot.h>
#include <kernel/debug.h>
#include <kernel/serial.h>

/**
 * Test terminal scrolling functionality
 */
void test_scroll_functionality() {
    printf("\n=== Testing Terminal Scrolling ===\n");

    // Generate enough lines to test scrolling behavior
    for (size_t i = 0; i < 30; i++) {
        printf("Scroll test line %zu of 30\n", i + 1);
    }

    printf("Scroll test complete\n");
}

/**
 * Kernel entry point
 */
void kernel_main(unsigned long magic, unsigned long addr) {
    assert(magic == MULTIBOOT_BOOTLOADER_MAGIC && "Invalid bootloader magic");
    multiboot_info_t* mbi = (multiboot_info_t*) addr;
    debug_initialize(mbi);

    terminal_initialize();
    printf("=======================================\n");
    printf("Welcome to Olympos\n");
    printf("An experimental 32-bit Operating System\n");
    printf("=======================================\n");
}
