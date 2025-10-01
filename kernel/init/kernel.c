#include <stdio.h>
#include <stddef.h>
#include <assert.h>

#include <kernel/tty.h>
#include <kernel/multiboot.h>
#include <kernel/debug.h>
#include <kernel/gdt.h>
#include <kernel/interrupts.h>
#include <kernel/keyboard.h>

/**
 * Kernel entry point
 */
void kernel_main(unsigned long magic, unsigned long addr) {
    assert(magic == MULTIBOOT_BOOTLOADER_MAGIC && "Invalid bootloader magic");
    multiboot_info_t* mbi = (multiboot_info_t*) addr;
    debug_initialize(mbi);

    terminal_initialize();
    gdt_init();
    idt_init();
    keyboard_initialize();

    printf("=======================================\n");
    printf("Welcome to Olympos\n");
    printf("An experimental 32-bit Operating System\n");
    printf("=======================================\n");

    /* Enter idle loop. CPU halts until woken by interrupts (keyboard, timer, etc.). */
    while (1) {
        asm volatile("hlt");
    }
}
