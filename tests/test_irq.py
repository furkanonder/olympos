from test_framework import OlymposTestFramework

IRQ_TEST_TEMPLATE = """
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


def register_irq_tests(framework: OlymposTestFramework):
    # Test 1: IRQ registration basic functionality
    test_body = """
    printf("TEST_RUNNING\\n");
    
    printf("Testing IRQ registration...\\n");
    
    // Handler function
    void test_irq_handler(regs_t* r) {{
        printf("IRQ handler called\\n");
    }}
    
    // Register IRQ 1 (keyboard)
    int result = reqister_irq(1, test_irq_handler);
    
    if (result == 0) {{
        printf("IRQ 1 registered successfully\\n");
        printf("TEST_PASS\\n");
    }}
    else {{
        printf("IRQ registration failed\\n");
        printf("TEST_FAIL\\n");
    }}
    """
    
    framework.register_test(
        name="irq_registration_basic",
        test_code=IRQ_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 2: Multiple IRQ registration
    test_body = """
    printf("TEST_RUNNING\\n");
    
    printf("Testing multiple IRQ registrations...\\n");
    
    void handler1(regs_t* r) {{ printf("Handler 1\\n"); }}
    void handler2(regs_t* r) {{ printf("Handler 2\\n"); }}
    void handler3(regs_t* r) {{ printf("Handler 3\\n"); }}
    
    int r1 = reqister_irq(0, handler1);  // Timer
    int r2 = reqister_irq(1, handler2);  // Keyboard
    int r3 = reqister_irq(4, handler3);  // COM1
    
    if (r1 == 0 && r2 == 0 && r3 == 0) {{
        printf("Multiple IRQ registrations successful\\n");
        printf("TEST_PASS\\n");
    }}
    else {{
        printf("Multiple IRQ registration failed\\n");
        printf("TEST_FAIL\\n");
    }}
    """
    
    framework.register_test(
        name="irq_registration_multiple",
        test_code=IRQ_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 3: IRQ registration bounds checking
    test_body = """
    printf("TEST_RUNNING\\n");
    
    printf("Testing IRQ registration bounds...\\n");
    
    void test_handler(regs_t* r) {{ printf("Handler\\n"); }}
    
    // Test invalid IRQ numbers
    int r1 = reqister_irq(-1, test_handler);   // Should fail (< 0)
    int r2 = reqister_irq(16, test_handler);   // Should fail (>= 16)
    int r3 = reqister_irq(100, test_handler);  // Should fail (>= 16)
    
    // Test valid IRQ numbers
    int r4 = reqister_irq(0, test_handler);    // Should succeed
    int r5 = reqister_irq(15, test_handler);   // Should succeed
    
    if (r1 == -1 && r2 == -1 && r3 == -1 && r4 == 0 && r5 == 0) {{
        printf("IRQ bounds checking correct\\n");
        printf("TEST_PASS\\n");
    }}
    else {{
        printf("IRQ bounds checking failed\\n");
        printf("TEST_FAIL\\n");
    }}
    """
    
    framework.register_test(
        name="irq_registration_bounds",
        test_code=IRQ_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 4: IRQ unregistration
    test_body = """
    printf("TEST_RUNNING\\n");
    
    printf("Testing IRQ unregistration...\\n");
    
    void test_handler(regs_t* r) {{ printf("Handler\\n"); }}
    
    // Register IRQ 1
    int r1 = reqister_irq(1, test_handler);
    printf("Registered IRQ 1: %d\\n", r1);
    
    // Unregister IRQ 1
    int r2 = unregister_irq(1);
    printf("Unregistered IRQ 1: %d\\n", r2);
    
    if (r1 == 0 && r2 == 0) {{
        printf("IRQ unregistration successful\\n");
        printf("TEST_PASS\\n");
    }}
    else {{
        printf("IRQ unregistration failed\\n");
        printf("TEST_FAIL\\n");
    }}
    """
    
    framework.register_test(
        name="irq_unregistration",
        test_code=IRQ_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 5: IRQ unregistration bounds checking
    test_body = """
    printf("TEST_RUNNING\\n");
    
    printf("Testing IRQ unregistration bounds...\\n");
    
    // Test invalid IRQ numbers for unregister
    int r1 = unregister_irq(-1);   // Should fail
    int r2 = unregister_irq(16);   // Should fail
    int r3 = unregister_irq(100);  // Should fail
    
    // Test valid IRQ numbers
    int r4 = unregister_irq(0);    // Should succeed
    int r5 = unregister_irq(15);   // Should succeed
    
    if (r1 == -1 && r2 == -1 && r3 == -1 && r4 == 0 && r5 == 0) {{
        printf("IRQ unregistration bounds checking correct\\n");
        printf("TEST_PASS\\n");
    }}
    else {{
        printf("IRQ unregistration bounds checking failed\\n");
        printf("TEST_FAIL\\n");
    }}
    """
    
    framework.register_test(
        name="irq_unregistration_bounds",
        test_code=IRQ_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 6: Register, unregister, re-register cycle
    test_body = """
    printf("TEST_RUNNING\\n");
    
    printf("Testing IRQ register/unregister cycle...\\n");
    
    void handler1(regs_t* r) {{ printf("Handler 1\\n"); }}
    void handler2(regs_t* r) {{ printf("Handler 2\\n"); }}
    
    // Register IRQ 1 with handler1
    int r1 = reqister_irq(1, handler1);
    printf("Registered IRQ 1 with handler1: %d\\n", r1);
    
    // Unregister IRQ 1
    int r2 = unregister_irq(1);
    printf("Unregistered IRQ 1: %d\\n", r2);
    
    // Re-register IRQ 1 with handler2
    int r3 = reqister_irq(1, handler2);
    printf("Re-registered IRQ 1 with handler2: %d\\n", r3);
    
    if (r1 == 0 && r2 == 0 && r3 == 0) {{
        printf("IRQ register/unregister cycle successful\\n");
        printf("TEST_PASS\\n");
    }}
    else {{
        printf("IRQ register/unregister cycle failed\\n");
        printf("TEST_FAIL\\n");
    }}
    """
    
    framework.register_test(
        name="irq_register_cycle",
        test_code=IRQ_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 7: Register all 16 IRQs
    test_body = """
    printf("TEST_RUNNING\\n");
    
    printf("Testing registration of all 16 IRQs...\\n");
    
    void generic_handler(regs_t* r) {{ printf("IRQ Handler\\n"); }}
    
    int success = 1;
    
    // Register all 16 IRQs
    for (int i = 0; i < 16; i++) {{
        int result = reqister_irq(i, generic_handler);
        if (result != 0) {{
            printf("Failed to register IRQ %d\\n", i);
            success = 0;
        }}
    }}
    
    if (success) {{
        printf("All 16 IRQs registered successfully\\n");
        printf("TEST_PASS\\n");
    }}
    else {{
        printf("Failed to register all IRQs\\n");
        printf("TEST_FAIL\\n");
    }}
    """
    
    framework.register_test(
        name="irq_register_all",
        test_code=IRQ_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 8: Unregister all 16 IRQs
    test_body = """
    printf("TEST_RUNNING\\n");
    
    printf("Testing unregistration of all 16 IRQs...\\n");
    
    void generic_handler(regs_t* r) {{ printf("IRQ Handler\\n"); }}
    
    // First register all
    for (int i = 0; i < 16; i++) {{
        reqister_irq(i, generic_handler);
    }}
    printf("Registered all 16 IRQs\\n");
    
    // Then unregister all
    int success = 1;
    for (int i = 0; i < 16; i++) {{
        int result = unregister_irq(i);
        if (result != 0) {{
            printf("Failed to unregister IRQ %d\\n", i);
            success = 0;
        }}
    }}
    
    if (success) {{
        printf("All 16 IRQs unregistered successfully\\n");
        printf("TEST_PASS\\n");
    }}
    else {{
        printf("Failed to unregister all IRQs\\n");
        printf("TEST_FAIL\\n");
    }}
    """
    
    framework.register_test(
        name="irq_unregister_all",
        test_code=IRQ_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )


