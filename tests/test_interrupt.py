from test_framework import OlymposTestFramework

INTERRUPT_TEST_TEMPLATE = """
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include <kernel/tty.h>
#include <kernel/gdt.h>
#include <kernel/interrupts.h>

// Exit QEMU function
void exit_qemu(uint32_t exit_code) {{
    asm volatile("outl %0, %1" : : "a"(exit_code), "Nd"((uint16_t)0xf4));
}}

void kernel_main(unsigned long magic, unsigned long addr) {{
    terminal_initialize();
    gdt_init();
    idt_init();

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


def register_interrupt_tests(framework: OlymposTestFramework):
    # Test 1: Basic interrupt system initialization
    test_body = """
    printf("TEST_RUNNING\\n");

    // Test that interrupt system is initialized
    printf("Testing interrupt system initialization...\\n");

    // Check if we can register a handler (this tests the registration system)
    void test_handler(regs_t* r) {
        printf("Test handler called\\n");
    }

    int result = register_isr(0, test_handler);
    if (result == 0) {
        printf("ISR registration successful\\n");
        printf("TEST_PASS\\n");
    }
    else {
        printf("ISR registration failed\\n");
        printf("TEST_FAIL\\n");
    }
    """

    framework.register_test(
        name="interrupt_system_init",
        test_code=INTERRUPT_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 2: Division By Zero exception (Exception 0) - with custom handler
    test_body = """
    printf("TEST_RUNNING\\n");

    // Custom handler for Division By Zero
    void division_by_zero_handler(regs_t* r) {
        printf("Division By Zero exception caught!\\n");
        printf("Exception number: %u\\n", r->int_no);
        printf("TEST_PASS\\n");
        exit_qemu(0);
    }

    // Register the handler
    register_isr(0, division_by_zero_handler);

    printf("Testing Division By Zero exception...\\n");

    // Trigger Division By Zero
    volatile int a = 10;
    volatile int b = 0;
    volatile int result = a / b;  // This should trigger exception 0

    // If we reach here, the exception wasn't caught
    printf("ERROR: Division by zero not caught!\\n");
    printf("TEST_FAIL\\n");
    """

    framework.register_test(
        name="interrupt_division_by_zero",
        test_code=INTERRUPT_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 3: Multiple handler registration
    test_body = """
    printf("TEST_RUNNING\\n");

    // Test registering multiple handlers
    void handler1(regs_t* r) {
        printf("Handler 1 called\\n");
    }
    void handler2(regs_t* r) {
        printf("Handler 2 called\\n");
    }

    int result1 = register_isr(0, handler1);
    int result2 = register_isr(1, handler2);
    int result3 = register_isr(31, handler1);

    if (result1 == 0 && result2 == 0 && result3 == 0) {
        printf("Multiple handler registration successful\\n");
        printf("TEST_PASS\\n");
    }
    else {
        printf("Multiple handler registration failed\\n");
        printf("TEST_FAIL\\n");
    }
    """

    framework.register_test(
        name="interrupt_multiple_handlers",
        test_code=INTERRUPT_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 4: Invalid ISR number handling
    test_body = """
    printf("TEST_RUNNING\\n");

    void test_handler(regs_t* r) {
        printf("Handler called\\n");
    }

    // Test invalid ISR numbers
    int result1 = register_isr(-1, test_handler);  // Should fail
    int result2 = register_isr(32, test_handler);  // Should fail
    int result3 = register_isr(100, test_handler); // Should fail

    if (result1 == -1 && result2 == -1 && result3 == -1) {
        printf("Invalid ISR number handling correct\\n");
        printf("TEST_PASS\\n");
    }
    else {
        printf("Invalid ISR number handling incorrect\\n");
        printf("TEST_FAIL\\n");
    }
    """

    framework.register_test(
        name="interrupt_invalid_isr",
        test_code=INTERRUPT_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )
