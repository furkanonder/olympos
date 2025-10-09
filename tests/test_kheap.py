from test_framework import OlymposTestFramework

KHEAP_TEST_TEMPLATE = """
#include <kernel/serial.h>
#include <kernel/paging.h>
#include <kernel/kheap.h>
#include <kernel/multiboot.h>
#include <kernel/debug.h>
#include <stdint.h>
#include <string.h>

// Exit QEMU function
void exit_qemu(uint32_t exit_code) {{
    asm volatile("outl %0, %1" : : "a"(exit_code), "Nd"((uint16_t)0xf4));
}}

void kernel_main(unsigned long magic, unsigned long addr) {{
    // Initialize serial port
    serial_setup(SERIAL_COM1_BASE, SERIAL_BAUD_115200);
    serial_write_string(SERIAL_COM1_BASE, "Heap allocator test starting...\\n");

    // Initialize debug and paging (required before heap)
    multiboot_info_t* mbi = (multiboot_info_t*) addr;
    debug_initialize(mbi);
    paging_init();

    serial_write_string(SERIAL_COM1_BASE, "Initializing heap allocator...\\n");
    kheap_init();
    serial_write_string(SERIAL_COM1_BASE, "Heap initialized.\\n\\n");

    // Test code
    {test_body}

    serial_write_string(SERIAL_COM1_BASE, "TEST_PASS\\n");

    // Exit QEMU with success
    exit_qemu(0);

    // Halt if exit failed
    while(1) {{
        asm volatile("hlt");
    }}
}}
"""


def register_kheap_tests(framework: OlymposTestFramework):
    # Test 1: Basic allocation
    test_body = """
    serial_write_string(SERIAL_COM1_BASE, "Testing basic allocation...\\n");

    void* ptr1 = kmalloc(64);
    if (ptr1 == 0) {
        serial_write_string(SERIAL_COM1_BASE, "ERROR: kmalloc(64) returned NULL!\\n");
        exit_qemu(1);
    }
    serial_write_string(SERIAL_COM1_BASE, "kmalloc(64) success\\n");

    void* ptr2 = kmalloc(128);
    if (ptr2 == 0) {
        serial_write_string(SERIAL_COM1_BASE, "ERROR: kmalloc(128) returned NULL!\\n");
        exit_qemu(1);
    }
    serial_write_string(SERIAL_COM1_BASE, "kmalloc(128) success\\n");

    // Pointers should be different
    if (ptr1 == ptr2) {
        serial_write_string(SERIAL_COM1_BASE, "ERROR: Duplicate pointers!\\n");
        exit_qemu(1);
    }
    serial_write_string(SERIAL_COM1_BASE, "Pointers are unique\\n");
    """

    framework.register_test(
        name="kheap_basic_alloc",
        test_code=KHEAP_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS"
    )

    # Test 2: Allocation and deallocation
    test_body = """
    serial_write_string(SERIAL_COM1_BASE, "Testing allocation and deallocation...\\n");

    void* ptr1 = kmalloc(100);
    void* ptr2 = kmalloc(200);
    void* ptr3 = kmalloc(300);

    if (!ptr1 || !ptr2 || !ptr3) {
        serial_write_string(SERIAL_COM1_BASE, "ERROR: Allocation failed!\\n");
        exit_qemu(1);
    }
    serial_write_string(SERIAL_COM1_BASE, "All allocations successful\\n");

    // Free the middle one
    kfree(ptr2);
    serial_write_string(SERIAL_COM1_BASE, "Freed middle allocation\\n");

    // Allocate again - should reuse space
    void* ptr4 = kmalloc(150);
    if (!ptr4) {
        serial_write_string(SERIAL_COM1_BASE, "ERROR: Reallocation failed!\\n");
        exit_qemu(1);
    }
    serial_write_string(SERIAL_COM1_BASE, "Reallocation successful\\n");

    // Free all
    kfree(ptr1);
    kfree(ptr3);
    kfree(ptr4);
    serial_write_string(SERIAL_COM1_BASE, "All memory freed\\n");
    """

    framework.register_test(
        name="kheap_alloc_free",
        test_code=KHEAP_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS"
    )

    # Test 3: Memory operations
    test_body = """
    serial_write_string(SERIAL_COM1_BASE, "Testing memory operations...\\n");

    char* str1 = (char*) kmalloc(100);
    if (!str1) {
        serial_write_string(SERIAL_COM1_BASE, "ERROR: Allocation failed!\\n");
        exit_qemu(1);
    }

    // Write to allocated memory
    const char* test_str = "Hello, heap allocator!";
    for (int i = 0; test_str[i] != '\\0'; i++) {
        str1[i] = test_str[i];
    }
    str1[22] = '\\0';

    serial_write_string(SERIAL_COM1_BASE, "Wrote string to heap: ");
    serial_write_string(SERIAL_COM1_BASE, str1);
    serial_write_string(SERIAL_COM1_BASE, "\\n");

    // Verify data integrity
    int match = 1;
    for (int i = 0; test_str[i] != '\\0'; i++) {
        if (str1[i] != test_str[i]) {
            match = 0;
            break;
        }
    }

    if (!match) {
        serial_write_string(SERIAL_COM1_BASE, "ERROR: Data corruption!\\n");
        exit_qemu(1);
    }
    serial_write_string(SERIAL_COM1_BASE, "Data integrity verified\\n");

    kfree(str1);
    """

    framework.register_test(
        name="kheap_memory_ops",
        test_code=KHEAP_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS"
    )

    # Test 4: Multiple allocations and frees (stress test)
    test_body = """
    serial_write_string(SERIAL_COM1_BASE, "Testing multiple allocations...\\n");

    #define NUM_ALLOCS 10
    void* ptrs[NUM_ALLOCS];

    // Allocate multiple blocks
    for (int i = 0; i < NUM_ALLOCS; i++) {
        ptrs[i] = kmalloc((i + 1) * 50);
        if (!ptrs[i]) {
            serial_write_string(SERIAL_COM1_BASE, "ERROR: Allocation failed!\\n");
            exit_qemu(1);
        }
    }
    serial_write_string(SERIAL_COM1_BASE, "All allocations successful\\n");

    // Free every other block
    for (int i = 0; i < NUM_ALLOCS; i += 2) {
        kfree(ptrs[i]);
        ptrs[i] = 0;
    }
    serial_write_string(SERIAL_COM1_BASE, "Freed every other block\\n");

    // Free remaining blocks
    for (int i = 1; i < NUM_ALLOCS; i += 2) {
        kfree(ptrs[i]);
        ptrs[i] = 0;
    }
    serial_write_string(SERIAL_COM1_BASE, "Freed all blocks\\n");

    // Allocate a large block (should work due to coalescing)
    void* large = kmalloc(2000);
    if (!large) {
        serial_write_string(SERIAL_COM1_BASE, "ERROR: Large allocation failed after coalescing!\\n");
        exit_qemu(1);
    }
    serial_write_string(SERIAL_COM1_BASE, "Large allocation successful (coalescing works!)\\n");

    kfree(large);
    """

    framework.register_test(
        name="kheap_stress",
        test_code=KHEAP_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS"
    )

    # Test 5: Edge cases
    test_body = """
    serial_write_string(SERIAL_COM1_BASE, "Testing edge cases...\\n");

    // Test zero-size allocation
    void* ptr_zero = kmalloc(0);
    if (ptr_zero != 0) {
        serial_write_string(SERIAL_COM1_BASE, "WARNING: kmalloc(0) returned non-NULL\\n");
    } else {
        serial_write_string(SERIAL_COM1_BASE, "kmalloc(0) correctly returns NULL\\n");
    }

    // Test NULL free (should not crash)
    kfree(0);
    serial_write_string(SERIAL_COM1_BASE, "kfree(NULL) handled correctly\\n");

    // Test small allocation
    void* ptr_small = kmalloc(1);
    if (!ptr_small) {
        serial_write_string(SERIAL_COM1_BASE, "ERROR: Small allocation failed!\\n");
        exit_qemu(1);
    }
    serial_write_string(SERIAL_COM1_BASE, "Small (1 byte) allocation successful\\n");
    kfree(ptr_small);

    // Test medium allocation
    void* ptr_medium = kmalloc(1024);
    if (!ptr_medium) {
        serial_write_string(SERIAL_COM1_BASE, "ERROR: Medium allocation failed!\\n");
        exit_qemu(1);
    }
    serial_write_string(SERIAL_COM1_BASE, "Medium (1 KiB) allocation successful\\n");
    kfree(ptr_medium);
    """

    framework.register_test(
        name="kheap_edge_cases",
        test_code=KHEAP_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS"
    )
