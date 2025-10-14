from test_framework import OlymposTestFramework

TSS_TEST_TEMPLATE = """
#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/gdt.h>

// Exit QEMU function
void exit_qemu(uint32_t exit_code) {{
    asm volatile("outl %0, %1" : : "a"(exit_code), "Nd"((uint16_t)0xf4));
}}

// External declarations for inspecting GDT and TSS
extern uint32_t stack_top;

void kernel_main(unsigned long magic, unsigned long addr) {{
    (void) magic;  // Suppress unused parameter warning
    (void) addr;   // Suppress unused parameter warning
    
    terminal_initialize();
    gdt_init();
    
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


def register_tss_tests(framework: OlymposTestFramework):
    # Test 1: Verify TSS is loaded in Task Register
    test_body = """
    printf("TEST_RUNNING\\n");
    
    uint16_t tr;
    asm volatile("str %0" : "=r"(tr));
    printf("Task Register (TR): 0x%04x\\n", tr);
    
    // TSS selector should be (SEGMENT_TSS << 3) = (5 << 3) = 0x28
    if (tr == 0x28) {
        printf("TSS loaded correctly\\n");
        printf("TEST_PASS\\n");
    }
    else {
        printf("TSS not loaded correctly (expected 0x28)\\n");
    }
    """

    framework.register_test(
        name="tss_loaded", test_code=TSS_TEST_TEMPLATE.format(test_body=test_body), expected_output="TEST_PASS"
    )

    # Test 2: Verify TSS descriptor in GDT
    test_body = """
    printf("TEST_RUNNING\\n");
    
    // Get GDT base and limit using gdt_register_t from kernel headers
    gdt_register_t gdtr;
    asm volatile("sgdt %0" : "=m"(gdtr));
    
    // Get TSS descriptor (index 5)
    gdt_entry_t* gdt = (gdt_entry_t*) gdtr.base;
    gdt_entry_t tss_desc = gdt[5];
    
    // Extract TSS base address from descriptor
    uint32_t tss_base = tss_desc.base_lo | ((uint32_t) tss_desc.base_mi << 16) | ((uint32_t) tss_desc.base_hi << 24);
    
    // Extract TSS limit
    uint32_t tss_limit = tss_desc.limit_lo | ((tss_desc.limit_hi_flags & 0x0F) << 16);
    
    // Extract access byte
    uint8_t access = tss_desc.access;
    
    // Verify TSS descriptor properties
    // Access byte should be 0x8B (Present=1, DPL=00, Type=1011 Busy 32-bit TSS)
    // Note: CPU automatically changes type from 0x9 (Available) to 0xB (Busy) when TSS is loaded
    int checks_passed = 1;
    
    if (access != 0x8B) {
        printf("ERROR: Access byte incorrect\\n");
        checks_passed = 0;
    }
    
    if (tss_base == 0) {
        printf("ERROR: TSS base address is NULL\\n");
        checks_passed = 0;
    }
    
    if (tss_limit != sizeof(tss_entry_t)) {
        printf("ERROR: TSS limit incorrect\\n");
        checks_passed = 0;
    }
    
    if (checks_passed) {
        printf("TSS descriptor valid\\n");
        printf("TEST_PASS\\n");
    }
    """

    framework.register_test(
        name="tss_descriptor_valid",
        test_code=TSS_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 3: Verify TSS contents (esp0, ss0)
    test_body = """
    printf("TEST_RUNNING\\n");
    
    // Get GDT base
    gdt_register_t gdtr;
    asm volatile("sgdt %0" : "=m"(gdtr));
    
    // Get TSS base address from descriptor
    gdt_entry_t* gdt = (gdt_entry_t*) gdtr.base;
    gdt_entry_t tss_desc = gdt[5];
    uint32_t tss_base = tss_desc.base_lo | (tss_desc.base_mi << 16) | (tss_desc.base_hi << 24);
    
    // Cast to TSS structure
    tss_entry_t* tss = (tss_entry_t*)tss_base;
    
    printf("TSS Contents:\\n");
    printf("esp0: 0x%08x\\n", tss->esp0);
    printf("ss0: 0x%08x\\n", tss->ss0);
    printf("iomap_base: 0x%04x\\n", tss->iomap_base);
    
    uint32_t expected_stack = (uint32_t) &stack_top;
    printf("Expected stack_top: 0x%x\\n", expected_stack);
    
    int checks_passed = 1;
    
    // ss0 should be KERNEL_DS (0x10)
    if (tss->ss0 != 0x10) {
        printf("ERROR: ss0 incorrect (expected 0x10)\\n");
        checks_passed = 0;
    }
    
    // esp0 should point to stack_top
    if (tss->esp0 != expected_stack) {
        printf("ERROR: esp0 incorrect\\n");
        checks_passed = 0;
    }
    
    // iomap_base should be set to sizeof(tss)
    if (tss->iomap_base != sizeof(tss_entry_t)) {
        printf("ERROR: iomap_base incorrect (expected %u)\\n", sizeof(tss_entry_t));
        checks_passed = 0;
    }
    
    if (checks_passed) {
        printf("TSS contents valid\\n");
        printf("TEST_PASS\\n");
    }
    """

    framework.register_test(
        name="tss_contents_valid", test_code=TSS_TEST_TEMPLATE.format(test_body=test_body), expected_output="TEST_PASS"
    )

    # Test 4: Verify all 6 GDT entries exist
    test_body = """
    printf("TEST_RUNNING\\n");
    
    gdt_register_t gdtr;
    asm volatile("sgdt %0" : "=m"(gdtr));
    
    // GDT limit should accommodate 6 entries (0 - 5)
    uint16_t expected_limit = (sizeof(gdt_entry_t) * 6) - 1;
    
    printf("GDT Limit: 0x%04x\\n", gdtr.boundary);
    printf("Expected Limit: 0x%04x\\n", expected_limit);
    
    if (gdtr.boundary == expected_limit) {
        printf("GDT contains 6 entries (Null, KCode, KData, UCode, UData, TSS)\\n");
        printf("TEST_PASS\\n");
    }
    else {
        printf("GDT entry count mismatch\\n");
    }
    """

    framework.register_test(
        name="gdt_has_six_entries",
        test_code=TSS_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )
