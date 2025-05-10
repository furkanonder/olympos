from test_framework import OlymposTestFramework

TERMINAL_TEST_TEMPLATE = """
#include <stdio.h>
#include <stdint.h>

#include <kernel/tty.h>
#include <kernel/serial.h>

// Exit QEMU function
void exit_qemu(uint32_t exit_code) {{
    asm volatile("outl %0, %1" : : "a"(exit_code), "Nd"((uint16_t)0xf4));
}}

void kernel_main(unsigned long magic, unsigned long addr) {{
    // Initialize terminal - this will also initialize VGA
    terminal_initialize();
    
    // Initialize serial for test output monitoring
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

def register_terminal_tests(framework: OlymposTestFramework):
    # Simplified Test: VGA Buffer Verification for Scrolling
    test_body = """
        serial_write_string(SERIAL_COM1_BASE, "TEST_RUNNING\\n");
    
        uint16_t* const VGA_BUFFER = (uint16_t*) 0xB8000;
    
        // First clear the screen
        for (int i = 0; i < 25; i++) {
            printf("\\n");
        }
    
        // Write a special character to the first line
        printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\\n");
    
        // Fill remaining lines with different content
        for (int i = 1; i < 24; i++) {
            printf("Line %d: BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\\n", i);
        }
    
        // Read the character at start of first line in VGA buffer
        char first_line_char = (char)(VGA_BUFFER[0] & 0xFF);
    
        char buffer[64];
        // Report pre-scroll state
        snprintf(buffer, sizeof(buffer), "First line character before scroll: %c", first_line_char);
        // Trigger a scroll
        serial_write_string(SERIAL_COM1_BASE, "Triggering scroll\\n");
        printf("This line should trigger scrolling\\n");
    
        // Check the character at start of first line again
        char new_first_line_char = (char)(VGA_BUFFER[0] & 0xFF);
    
        // Report post-scroll state
        snprintf(buffer, sizeof(buffer), "First line character after scroll: %c\\n", new_first_line_char);
        serial_write_string(SERIAL_COM1_BASE, buffer);
    
        // Verify first line has changed from A to B, indicating scroll worked
        if (first_line_char == 'A' && new_first_line_char == 'L') {
            serial_write_string(SERIAL_COM1_BASE, "Scroll verification succeeded: content moved up correctly\\n");
            serial_write_string(SERIAL_COM1_BASE, "TEST_PASS\\n");
        }
        else {
            snprintf(buffer, sizeof(buffer), "Scroll verification failed! Expected L, got: %c\\n", new_first_line_char);
            serial_write_string(SERIAL_COM1_BASE, buffer);
            serial_write_string(SERIAL_COM1_BASE, "TEST_FAILED\\n");
        }
        """

    framework.register_test(
        name="terminal_vga_buffer_verification",
        test_code=TERMINAL_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS"
    )