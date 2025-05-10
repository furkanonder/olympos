import os
import shutil

ASSERT_FILE_PATH = "libc/assert/assert.c"
MODIFIED_ASSERT_C = """
#include <stdio.h>

#include <kernel/debug.h>

__attribute__((__noreturn__))
void __assert_fail(const char *expr, const char *file, unsigned int line, const char *function) {
    printf("%s: %s:%d: %s: Assertion `%s' failed.\\n", "kernel", file, line, function, expr);
    print_backtrace();

    // In TEST mode, exit QEMU so tests can continue
    asm volatile("outl %0, %1" : : "a"(0), "Nd"((uint16_t)0xf4));
    
    // This will only be reached in non-test mode
    while (1) {
        asm volatile ("hlt");
    }
    __builtin_unreachable();
}
"""

# Template for assertion tests
ASSERT_TEST_TEMPLATE = """
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include <kernel/serial.h>

// Exit QEMU function for successful test completion
void exit_qemu(uint32_t exit_code) {{
    asm volatile("outl %0, %1" : : "a"(exit_code), "Nd"((uint16_t)0xf4));
}}

void kernel_main(unsigned long magic, unsigned long addr) {{
    // Initialize serial for testing
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


def backup_assert_file(framework):
    assert_path = framework.get_path(ASSERT_FILE_PATH)
    backup_path = f"{assert_path}.bak"
    if os.path.exists(backup_path):
        os.remove(backup_path)
    shutil.copy2(assert_path, backup_path)
    return backup_path


def modify_assert_file(framework):
    assert_path = framework.get_path(ASSERT_FILE_PATH)
    with open(assert_path, "w") as f:
        f.write(MODIFIED_ASSERT_C)


def restore_assert_file(framework, backup_path):
    assert_path = framework.get_path(ASSERT_FILE_PATH)
    if os.path.exists(backup_path):
        shutil.copy2(backup_path, assert_path)
        os.remove(backup_path)


def register_assert_tests(framework):
    original_run_all_tests = framework.run_all_tests
    def patched_run_all_tests():
        backup_path = backup_assert_file(framework)
        modify_assert_file(framework)
        try:
            return original_run_all_tests()
        finally:
            restore_assert_file(framework, backup_path)
    framework.run_all_tests = patched_run_all_tests

    # Test 1: Successful assertion
    test_body = """
    serial_write_string(SERIAL_COM1_BASE, "TEST_RUNNING\\n");
    
    int w = 5, y = 33;
    serial_write_string(SERIAL_COM1_BASE, "Asserting that sum > 30... ");
    assert((w + y) > 30 && "Sum is not greater than 30");
    serial_write_string(SERIAL_COM1_BASE, "Passed!\\n");
    
    serial_write_string(SERIAL_COM1_BASE, "TEST_PASS\\n");
    """

    framework.register_test(
        name="assert_success", test_code=ASSERT_TEST_TEMPLATE.format(test_body=test_body), expected_output="TEST_PASS"
    )

    # Test 2: Multiple successful assertions
    test_body = """
    serial_write_string(SERIAL_COM1_BASE, "TEST_RUNNING\\n");
    
    int w = 5, y = 33;
    serial_write_string(SERIAL_COM1_BASE, "Asserting that sum > 30... ");
    assert((w + y) > 30 && "Sum is not greater than 30");
    serial_write_string(SERIAL_COM1_BASE, "Passed!\\n");
    
    serial_write_string(SERIAL_COM1_BASE, "Asserting that sum > 37... ");
    assert((w + y) > 37 && "Sum is not greater than 37");
    serial_write_string(SERIAL_COM1_BASE, "Passed!\\n");
    
    serial_write_string(SERIAL_COM1_BASE, "TEST_PASS\\n");
    """

    framework.register_test(
        name="assert_multiple_success",
        test_code=ASSERT_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 3: Assert with edge values
    test_body = """
    serial_write_string(SERIAL_COM1_BASE, "TEST_RUNNING\\n");
    
    // Edge case: exactly equal
    int a = 38, b = 0;
    serial_write_string(SERIAL_COM1_BASE, "Testing edge case: exactly equal... ");
    assert((a + b) == 38 && "Equal check failed");
    serial_write_string(SERIAL_COM1_BASE, "Passed!\\n");
    
    // Edge case: boundary condition
    a = 2147483647;  // INT_MAX
    b = 0;
    serial_write_string(SERIAL_COM1_BASE, "Testing edge case: INT_MAX... ");
    assert(a > 0 && "INT_MAX should be positive");
    serial_write_string(SERIAL_COM1_BASE, "Passed!\\n");
    
    serial_write_string(SERIAL_COM1_BASE, "TEST_PASS\\n");
    """

    framework.register_test(
        name="assert_edge_values",
        test_code=ASSERT_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 4: Assert with complex expressions
    test_body = """
    serial_write_string(SERIAL_COM1_BASE, "TEST_RUNNING\\n");
    
    int a = 10, b = 20, c = 30;
    serial_write_string(SERIAL_COM1_BASE, "Testing complex assertion... ");
    assert(((a * b) > (c * 5)) && (a != 0) && (b != 0) && "Complex expression failed");
    serial_write_string(SERIAL_COM1_BASE, "Passed!\\n");
    
    serial_write_string(SERIAL_COM1_BASE, "TEST_PASS\\n");
    """

    framework.register_test(
        name="assert_complex_expression",
        test_code=ASSERT_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 5: Assert failure
    test_body = """
    int w = 5, y = 33;
    assert((w + y) > 50 && "Sum is not greater than 50");
    """

    framework.register_test(
        name="assert_failure",
        test_code=ASSERT_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="""kernel: init/kernel.c:20: kernel_main: Assertion `(w + y) > 50 && "Sum is not greater than 50"' failed.""",
    )
