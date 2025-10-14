from test_framework import OlymposTestFramework

SYSCALL_TEST_TEMPLATE = """
#include <stdio.h>

#include <kernel/interrupts.h>
#include <kernel/syscall.h>

// Exit QEMU function
void exit_qemu(uint32_t exit_code) {{
    asm volatile("outl %0, %1" : : "a"(exit_code), "Nd"((uint16_t)0xf4));
}}

void kernel_main(unsigned long magic, unsigned long addr) {{
    (void) magic;  // Suppress unused parameter warning
    (void) addr;   // Suppress unused parameter warning
    
    terminal_initialize();
    gdt_init();
    idt_init();
    syscall_init();
    
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


def register_syscall_tests(framework: OlymposTestFramework):
    # Test 1: Test syscall write functionality (from kernel mode)
    test_body = """
    printf("TEST_RUNNING\\n");
    
    // Simulate a syscall by calling the handler directly. In kernel mode, we can test the handler function
    const char* test_msg = "SYSCALL_WRITE_TEST";
    
    // Use the regs_t structure from interrupts.h
    regs_t regs;
    memset(&regs, 0, sizeof(regs));
    
    // Setup syscall arguments
    regs.eax = 4;                          // SYSCALL_WRITE
    regs.ebx = 1;                          // fd = stdout
    regs.ecx = (uint32_t) test_msg;        // buffer
    regs.edx = strlen(test_msg);           // count
    
    // Call syscall handler
    extern void syscall_handler(regs_t* r);
    syscall_handler(&regs);
        
    // Check return value (should be number of bytes written)
    if (regs.eax == strlen(test_msg)) {
        printf("Syscall write returned correct byte count\\n");
        printf("TEST_PASS\\n");
    }
    else {
        printf("Syscall write failed (returned %u, expected %u)\\n", regs.eax, strlen(test_msg));
    }
    """

    framework.register_test(
        name="syscall_write_handler",
        test_code=SYSCALL_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 2: Test syscall with invalid syscall number
    test_body = """
    printf("TEST_RUNNING\\n");
    
    regs_t regs;
    memset(&regs, 0, sizeof(regs));
    regs.eax = 999;  // Invalid syscall
    
    extern void syscall_handler(regs_t* r);
    syscall_handler(&regs);
    
    // Return value should be -1 (error)
    if ((int32_t) regs.eax == -1) {
        printf("Invalid syscall correctly returned error\\n");
        printf("TEST_PASS\\n");
    }
    else {
        printf("Invalid syscall did not return error\\n");
    }
    """

    framework.register_test(
        name="syscall_invalid_number",
        test_code=SYSCALL_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 3: Test syscall write with invalid file descriptor
    test_body = """
    printf("TEST_RUNNING\\n");
    
    regs_t regs;
    memset(&regs, 0, sizeof(regs));
    
    const char* test_msg = "test";
    regs.eax = 4;                    // SYSCALL_WRITE
    regs.ebx = 999;                  // Invalid fd
    regs.ecx = (uint32_t) test_msg;
    regs.edx = strlen(test_msg);
    
    extern void syscall_handler(regs_t* r);
    syscall_handler(&regs);
    
    // Return value should be -1 (error)
    if ((int32_t) regs.eax == -1) {
        printf("Write to invalid fd correctly returned error\\n");
        printf("TEST_PASS\\n");
    }
    else {
        printf("Write to invalid fd did not return error\\n");
    }
    """

    framework.register_test(
        name="syscall_write_invalid_fd",
        test_code=SYSCALL_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 4: Test syscall write to stderr (fd=2)
    test_body = """
    printf("TEST_RUNNING\\n");
    
    regs_t regs;
    memset(&regs, 0, sizeof(regs));
    
    const char* test_msg = "STDERR_TEST";
    regs.eax = 4;                          // SYSCALL_WRITE
    regs.ebx = 2;                          // fd = stderr
    regs.ecx = (uint32_t) test_msg;
    regs.edx = strlen(test_msg);
    
    extern void syscall_handler(regs_t* r);
    syscall_handler(&regs);
        
    if (regs.eax == strlen(test_msg)) {
        printf("Syscall write to stderr successful\\n");
        printf("TEST_PASS\\n");
    }
    else {
        printf("Syscall write to stderr failed\\n");
    }
    """

    framework.register_test(
        name="syscall_write_stderr",
        test_code=SYSCALL_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )
