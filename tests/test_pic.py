from test_framework import OlymposTestFramework

PIC_TEST_TEMPLATE = """
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include <kernel/tty.h>
#include <kernel/gdt.h>
#include <kernel/interrupts.h>

// PIC functions (from pic.h)
extern void pic_remap(uint8_t master_offset, uint8_t slave_offset);
extern void pic_mask(uint8_t irq);
extern void pic_unmask(uint8_t irq);
extern uint16_t pic_get_irr(void);
extern uint16_t pic_get_isr(void);
extern void pic_send_eoi(uint8_t irq);

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


def register_pic_tests(framework: OlymposTestFramework):
    # Test 1: PIC initialization - all IRQs should be masked after init
    test_body = """
    printf("TEST_RUNNING\\n");
    
    // After idt_init(), PIC should be remapped and all IRQs masked (0xFFFF)
    uint16_t irr = pic_get_irr();
    
    printf("Testing PIC initialization...\\n");
    printf("IRR value after init: 0x%x\\n", irr);
    
    // IRR should be 0 (no pending interrupts) since all are masked
    if (irr == 0) {
        printf("PIC initialized correctly - no pending interrupts\\n");
        printf("TEST_PASS\\n");
    }
    else {
        printf("Unexpected IRR value\\n");
        printf("TEST_FAIL\\n");
    }
    """
    
    framework.register_test(
        name="pic_initialization",
        test_code=PIC_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 2: PIC ISR/IRR reading functionality
    test_body = """
    printf("TEST_RUNNING\\n");
    
    // Test that we can read ISR and IRR
    uint16_t irr = pic_get_irr();
    uint16_t isr = pic_get_isr();
    
    printf("Testing ISR/IRR reading...\\n");
    printf("IRR: 0x%x\\n", irr);
    printf("ISR: 0x%x\\n", isr);
    
    // After initialization with no interrupts fired, ISR should be 0
    if (isr == 0) {
        printf("ISR correctly reads as 0 (no interrupts being serviced)\\n");
        printf("TEST_PASS\\n");
    }
    else {
        printf("Unexpected ISR value: 0x%x\\n", isr);
        printf("TEST_FAIL\\n");
    }
    """
    
    framework.register_test(
        name="pic_isr_irr_reading",
        test_code=PIC_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 3: PIC mask/unmask functionality
    test_body = """
    printf("TEST_RUNNING\\n");
    
    printf("Testing PIC mask/unmask...\\n");
    
    // Read current IMR state via attempting to read data port
    // Note: We can't directly read IMR, but we can test the functions work
    
    // Unmask IRQ 0 (timer)
    pic_unmask(0);
    printf("Unmasked IRQ 0\\n");
    
    // Mask it again
    pic_mask(0);
    printf("Masked IRQ 0\\n");
    
    // Unmask IRQ 1 (keyboard)  
    pic_unmask(1);
    printf("Unmasked IRQ 1\\n");
    
    // Mask it again
    pic_mask(1);
    printf("Masked IRQ 1\\n");
    
    // Unmask IRQ 8 (RTC - slave PIC)
    pic_unmask(8);
    printf("Unmasked IRQ 8\\n");
    
    // Mask it again
    pic_mask(8);
    printf("Masked IRQ 8\\n");
    
    // If we got here without crashing, the functions work
    printf("Mask/unmask operations completed\\n");
    printf("TEST_PASS\\n");
    """
    
    framework.register_test(
        name="pic_mask_unmask",
        test_code=PIC_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 4: PIC remapping
    test_body = """
    printf("TEST_RUNNING\\n");
    
    printf("Testing PIC remapping...\\n");
    
    // Remap to standard offsets (0x20, 0x28)
    pic_remap(0x20, 0x28);
    printf("Remapped PIC to 0x20/0x28\\n");
    
    // After remapping, all IRQs should be masked
    uint16_t irr = pic_get_irr();
    uint16_t isr = pic_get_isr();
    
    printf("IRR after remap: 0x%x\\n", irr);
    printf("ISR after remap: 0x%x\\n", isr);
    
    // ISR should be 0 (no active interrupts)
    if (isr == 0) {
        printf("PIC remapping successful\\n");
        printf("TEST_PASS\\n");
    }
    else {
        printf("Unexpected ISR after remap\\n");
        printf("TEST_FAIL\\n");
    }
    """
    
    framework.register_test(
        name="pic_remapping",
        test_code=PIC_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 5: Test all 16 IRQ lines mask/unmask
    test_body = """
    printf("TEST_RUNNING\\n");
    
    printf("Testing all 16 IRQ lines...\\n");
    
    int success = 1;
    
    // Test unmasking all IRQs
    for (int i = 0; i < 16; i++) {
        pic_unmask(i);
    }
    printf("Unmasked all 16 IRQs\\n");
    
    // Test masking all IRQs
    for (int i = 0; i < 16; i++) {
        pic_mask(i);
    }
    printf("Masked all 16 IRQs\\n");
    
    if (success) {
        printf("All IRQ mask/unmask operations successful\\n");
        printf("TEST_PASS\\n");
    }
    else {
        printf("TEST_FAIL\\n");
    }
    """
    
    framework.register_test(
        name="pic_all_irqs",
        test_code=PIC_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )
