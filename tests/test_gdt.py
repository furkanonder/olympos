from test_framework import OlymposTestFramework

GDT_TEST_TEMPLATE = """
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include <kernel/tty.h>
#include <kernel/gdt.h>

// Exit QEMU function
void exit_qemu(uint32_t exit_code) {{
    asm volatile("outl %0, %1" : : "a"(exit_code), "Nd"((uint16_t)0xf4));
}}

void kernel_main(unsigned long magic, unsigned long addr) {{
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


def register_gdt_tests(framework: OlymposTestFramework):
    # Test 1: GDT segments verification
    test_body = """
    printf("TEST_RUNNING\\n");
    
    uint16_t cs, ds, ss, es, fs, gs;
    asm volatile("mov %%cs, %0" : "=r"(cs));
    asm volatile("mov %%ds, %0" : "=r"(ds));
    asm volatile("mov %%ss, %0" : "=r"(ss));
    asm volatile("mov %%es, %0" : "=r"(es));
    asm volatile("mov %%fs, %0" : "=r"(fs));
    asm volatile("mov %%gs, %0" : "=r"(gs));
    
    printf("All segment registers:\\n");
    printf("  CS: 0x%04x\\n", cs);
    printf("  DS: 0x%04x\\n", ds);
    printf("  SS: 0x%04x\\n", ss);
    printf("  ES: 0x%04x\\n", es);
    printf("  FS: 0x%04x\\n", fs);
    printf("  GS: 0x%04x\\n", gs);
    
    if (cs == 0x08 && ds == 0x10 && ss == 0x10 && es == 0x10 && fs == 0x10 && gs == 0x10) {
        printf("All segments correctly set\\n");
        printf("TEST_PASS\\n");
    }
    else {
        printf("Some segments incorrect\\n");
    }
    """

    framework.register_test(
        name="gdt_all_segments", test_code=GDT_TEST_TEMPLATE.format(test_body=test_body), expected_output="TEST_PASS"
    )