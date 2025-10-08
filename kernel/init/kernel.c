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
    keyboard_initialize();

    printf("=======================================\n");
    printf("Welcome to Olympos\n");
    printf("An experimental 32-bit Operating System\n");
    printf("=======================================\n");

    printf("Supported physical memory size: %d MiB\n", NUM_FRAMES * 4 / 1024);
    printf("Reserved memory for the kernel: %d MiB\n", KMEM_MAX / 1024 / 1024);

    uint32_t faulty_addr = 0x800100;      // This address is NOT in kernel's mapped region
    int dummy = *((int*) faulty_addr);    // This triggers page fault (ISR #14)
    printf("Value: %d\n", dummy);         // This line will NEVER execute

    /* Enter idle loop. CPU halts until woken by interrupts (keyboard, timer, etc.). */
    while (1) {
        asm volatile("hlt");
    }
}
