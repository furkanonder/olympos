#include <stdio.h>
#include <stddef.h>
#include <assert.h>

#include <kernel/tty.h>
#include <kernel/multiboot.h>

void kernel_main(unsigned long magic, unsigned long addr) {
    terminal_initialize();
    printf("Welcome to Olympos!\n");
    printf("An experimental 32-bit Operating System\n");

    printf("\nTest printf function with different argument types:\n");

    char letter = 'F';
    printf("Character: %c\n", letter);

    const char *str = "Hello, World!\n";
    printf("String: %s", str);

    int a = 27;
    printf("Integer: %d\n", a);
    printf("Negative Integer: %d\n", -42);

    int x = 10;
    int *ptr = &x;
    printf("Address of x: %p\n", (void*)ptr);
    printf("Address of ptr: %p\n", (void*)&ptr);

    long int ld_val = 1234567890;
    printf("Long Integer (ld): %ld\n", ld_val);
    unsigned long int lu_val = 1234567890;
    printf("Unsigned Long Integer (lu): %lu\n", lu_val);
    unsigned long int lx_val = 1234567890;
    printf("Unsigned Long Integer in Hex (lx): %lx\n", lx_val);

    unsigned int hex_val = 0xff;
    printf("Hexadecimal (x): 0x%x\n", hex_val);

    // Test the assert
    int w = 5, y = 33;
    assert((w + y) > 50 && "Sum is not greater than 50");

    // Terminal Scroll Test
    for (size_t i = 0; i < 40; i++) {
        printf("Line: %d\n", i);
    }

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        panic("Invalid bootloader magic %lx\n", magic);
    }

}
