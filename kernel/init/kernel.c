#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <stdbool.h>

#include <kernel/tty.h>
#include <kernel/multiboot.h>
#include <kernel/debug.h>
#include <kernel/serial.h>

/**
 * Test assertion functionality
 * Note: Will trigger kernel panic on failed assertion
 */
void test_assertions() {
    printf("\n=== Testing Assertions ===\n");

    int w = 5, y = 33;
    printf("Asserting that %d + %d > 30... ", w, y);
    assert((w + y) > 30 && "Sum is not greater than 30");
    printf("Passed!\n");

    printf("Asserting that %d + %d > 38... ", w, y);
    assert((w + y) > 38 && "Sum is not greater than 38");
    printf("Passed!\n");

    // This should fail and trigger a panic
    printf("Asserting that %d + %d > 50... ", w, y);
    assert((w + y) > 50 && "Sum is not greater than 50");
    printf("Passed! (You should not see this message)\n");
}

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
 * Test various printf format specifiers
 */
void test_printf_arguments() {
    printf("\n=== Testing Printf Format Specifiers ===\n");

    // Character test
    char letter = 'F';
    printf("Character (%%c): %c\n", letter);

    // String test
    const char *str = "Hello, World!";
    printf("String (%%s): %s\n", str);

    // Integer tests
    int a = 27;
    printf("Integer (%%d): %d\n", a);
    printf("Negative Integer (%%d): %d\n", -42);

    // Pointer tests
    int x = 10;
    int *ptr = &x;
    printf("Address of x (%%p): %p\n", (void*)ptr);
    printf("Address of ptr (%%p): %p\n", (void*)&ptr);

    // Long integer tests
    long int ld_val = 1234567890;
    printf("Long Integer (%%ld): %ld\n", ld_val);
    unsigned long int lu_val = 1234567890;
    printf("Unsigned Long Integer (%%lu): %lu\n", lu_val);
    unsigned long int lx_val = 0xABCD1234;
    printf("Unsigned Long Integer in Hex (%%lx): 0x%lx\n", lx_val);

    // Hexadecimal test
    unsigned int hex_val = 0xFF;
    printf("Hexadecimal (%%x): 0x%x\n", hex_val);

    // Size_t tests
    size_t size_val = 123456;
    printf("Size_t value (%%zu): %zu\n", size_val);
    printf("Size_t value (%%zd): %zd\n", size_val);

    // Combination test
    printf("Combined formats: char=%c, int=%d, hex=0x%x, size_t=%zu\n", 'X', 42, 0xDEAD, (size_t)98765);
}

/**
 * Main test suite - runs all or selected tests
 *
 * @param run_safe_only If true, only runs tests that shouldn't crash
 */
void run_test_suite(bool run_safe_only) {
    printf("\n======================================\n");
    printf("       OLYMPOS OS TEST SUITE          \n");
    printf("======================================\n");

    // Always run safe tests
    test_printf_arguments();

    // Only run potentially system-crashing tests if requested
    if (!run_safe_only) {
        printf("\nWARNING: Running tests that may crash the system!\n");
        test_scroll_functionality();
        test_assertions();
    }

    printf("\n=== Test Suite Completed Successfully ===\n");
}

/**
 * Kernel entry point
 */
void kernel_main(unsigned long magic, unsigned long addr) {
    assert(magic == MULTIBOOT_BOOTLOADER_MAGIC && "Invalid bootloader magic");
    multiboot_info_t* mbi = (multiboot_info_t*) addr;
    debug_initialize(mbi);

    terminal_initialize();
    // Print welcome message
    printf("=======================================\n");
    printf("Welcome to Olympos OS v0.1\n");
    printf("An experimental 32-bit Operating System\n");
    printf("=======================================\n");

    // Test serial driver
    serial_initialize(SERIAL_COM1_BASE, SERIAL_BAUD_115200);
    serial_write_string(SERIAL_COM1_BASE, "Serial Test\n");

    // Run test suite with only safe tests
    // Change to false to run all tests including ones that may crash
    run_test_suite(true);
}
