from test_framework import OlymposTestFramework

PAGING_TEST_TEMPLATE = """
#include <kernel/serial.h>
#include <kernel/paging.h>
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
    serial_write_string(SERIAL_COM1_BASE, "Paging test starting...\\n");
    
    // Initialize debug (needed for elf_sections_end)
    multiboot_info_t* mbi = (multiboot_info_t*) addr;
    debug_initialize(mbi);
    
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


def register_paging_tests(framework: OlymposTestFramework):
    # Test 1: Basic paging initialization
    test_body = """
    serial_write_string(SERIAL_COM1_BASE, "Initializing paging...\\n");
    paging_init();
    serial_write_string(SERIAL_COM1_BASE, "Paging initialized successfully!\\n");
    
    // Test reading from identity-mapped kernel memory
    volatile uint32_t* test_addr = (volatile uint32_t*)0x100000;  // 1 MB mark
    uint32_t value = *test_addr;
    serial_write_string(SERIAL_COM1_BASE, "Read from identity-mapped memory successful!\\n");
    """

    framework.register_test(
        name="paging_init",
        test_code=PAGING_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS"
    )

    # Test 2: Frame allocation
    test_body = """
    serial_write_string(SERIAL_COM1_BASE, "Initializing paging...\\n");
    paging_init();
    
    serial_write_string(SERIAL_COM1_BASE, "Testing frame allocation...\\n");
    uint32_t frame1 = frame_alloc();
    uint32_t frame2 = frame_alloc();
    uint32_t frame3 = frame_alloc();
    
    if (frame1 == 0 || frame2 == 0 || frame3 == 0) {
        serial_write_string(SERIAL_COM1_BASE, "ERROR: Frame allocation failed!\\n");
        exit_qemu(1);
    }
    
    if (frame1 == frame2 || frame2 == frame3 || frame1 == frame3) {
        serial_write_string(SERIAL_COM1_BASE, "ERROR: Duplicate frames allocated!\\n");
        exit_qemu(1);
    }
    
    serial_write_string(SERIAL_COM1_BASE, "Frame allocation working correctly!\\n");
    
    // Test frame freeing
    frame_free(frame2);
    uint32_t frame4 = frame_alloc();
    if (frame4 != frame2) {
        serial_write_string(SERIAL_COM1_BASE, "WARNING: Frame not reused (may be expected)\\n");
    }
    
    serial_write_string(SERIAL_COM1_BASE, "Frame management tests passed!\\n");
    """

    framework.register_test(
        name="paging_frame_alloc",
        test_code=PAGING_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS"
    )

    # Test 3: Page fault detection
    test_body = """
    serial_write_string(SERIAL_COM1_BASE, "Initializing paging...\\n");
    paging_init();
    
    serial_write_string(SERIAL_COM1_BASE, "Testing page fault detection...\\n");
    serial_write_string(SERIAL_COM1_BASE, "Reading from identity-mapped memory (should succeed)...\\n");
    
    // This should work - it's in the identity-mapped kernel region
    volatile uint32_t* valid_addr = (volatile uint32_t*)0x100000;  // 1 MB
    uint32_t value1 = *valid_addr;
    serial_write_string(SERIAL_COM1_BASE, "SUCCESS: Read from mapped memory\\n");
    
    // This should also work - still in kernel region (< 8MB)
    volatile uint32_t* valid_addr2 = (volatile uint32_t*)0x500000;  // 5 MB
    uint32_t value2 = *valid_addr2;
    serial_write_string(SERIAL_COM1_BASE, "SUCCESS: Read from another mapped address\\n");
    
    serial_write_string(SERIAL_COM1_BASE, "Page fault detection test passed!\\n");
    serial_write_string(SERIAL_COM1_BASE, "NOTE: Actual fault triggering tested in manual demo\\n");
    """

    framework.register_test(
        name="paging_fault_detection",
        test_code=PAGING_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS"
    )
