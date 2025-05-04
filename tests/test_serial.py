from test_framework import OlymposTestFramework

SERIAL_TEST_TEMPLATE = """
#include <kernel/serial.h>
#include <stdint.h>

// Exit QEMU function
void exit_qemu(uint32_t exit_code) {{
    asm volatile("outl %0, %1" : : "a"(exit_code), "Nd"((uint16_t)0xf4));
}}

void kernel_main(unsigned long magic, unsigned long addr) {{
    // Initialize serial port
    serial_setup(SERIAL_COM1_BASE, SERIAL_BAUD_115200);
    
    // Test code
    {test_body}
    
    // Exit QEMU with success
    exit_qemu(0);
    
    // Halt if exit failed
    while(1) {{
        asm volatile("hlt");
    }}
}}
"""


def register_serial_tests(framework: OlymposTestFramework):
    # Test 1: Basic serial output
    test_body = """
    serial_write_string(SERIAL_COM1_BASE, "TEST_RUNNING\\n");
    serial_write_char(SERIAL_COM1_BASE, 'X');
    serial_write_string(SERIAL_COM1_BASE, "\\n");
    serial_write_string(SERIAL_COM1_BASE, "TEST_PASS\\n");
    """

    framework.register_test(
        name="serial_basic", test_code=SERIAL_TEST_TEMPLATE.format(test_body=test_body), expected_output="TEST_PASS"
    )

    # Test 2: Special characters
    test_body = """
    serial_write_string(SERIAL_COM1_BASE, "Special chars: !@#$%^&*()\\n");
    serial_write_string(SERIAL_COM1_BASE, "TEST_PASS\\n");
    """

    framework.register_test(
        name="serial_special_chars",
        test_code=SERIAL_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 3: Long string
    test_body = """
    serial_write_string(SERIAL_COM1_BASE, "This is a long string to test buffer handling in the serial driver. ");
    serial_write_string(SERIAL_COM1_BASE, "It should handle multiple consecutive writes without issues.\\n");
    serial_write_string(SERIAL_COM1_BASE, "TEST_PASS\\n");
    """

    framework.register_test(
        name="serial_long_string",
        test_code=SERIAL_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )
