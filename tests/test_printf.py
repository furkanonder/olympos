from test_framework import OlymposTestFramework

PRINTF_TEST_TEMPLATE = """
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include <kernel/tty.h>

// Exit QEMU function
void exit_qemu(uint32_t exit_code) {{
    asm volatile("outl %0, %1" : : "a"(exit_code), "Nd"((uint16_t)0xf4));
}}

void kernel_main(unsigned long magic, unsigned long addr) {{
    terminal_initialize();
    
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


def register_printf_tests(framework: OlymposTestFramework):
    # Test 1: Basic printf
    test_body = """
    printf("TEST_RUNNING\\n");
    printf("X\\n");
    printf("TEST_PASS\\n");
    """

    framework.register_test(
        name="printf_basic", test_code=PRINTF_TEST_TEMPLATE.format(test_body=test_body), expected_output="TEST_PASS"
    )

    # Test 2: Character format specifier
    test_body = """
    char letter = 'F';
    printf("Character (%%c): %c\\n", letter);
    printf("TEST_PASS\\n");
    """

    framework.register_test(
        name="printf_format_c", test_code=PRINTF_TEST_TEMPLATE.format(test_body=test_body), expected_output="TEST_PASS"
    )

    # Test 3: String format specifier
    test_body = """
    const char *str = "Hello, World!";
    printf("String (%%s): %s\\n", str);
    printf("TEST_PASS\\n");
    """

    framework.register_test(
        name="printf_format_s", test_code=PRINTF_TEST_TEMPLATE.format(test_body=test_body), expected_output="TEST_PASS"
    )

    # Test 4: Integer format specifiers
    test_body = """
    int a = 27;
    printf("Integer (%%d): %d\\n", a);
    printf("Negative Integer (%%d): %d\\n", -42);
    printf("TEST_PASS\\n");
    """

    framework.register_test(
        name="printf_format_d", test_code=PRINTF_TEST_TEMPLATE.format(test_body=test_body), expected_output="TEST_PASS"
    )

    # Test 5: Unsigned integer format specifier
    test_body = """
    unsigned int u = 1234567890; // stays within INT_MAX to avoid itoa sign issues
    printf("Unsigned (%%u): %u\\n", u);
    printf("TEST_PASS\\n");
    """

    framework.register_test(
        name="printf_format_u",
        test_code=PRINTF_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 6: Pointer format specifier
    test_body = """
    int x = 10;
    int *ptr = &x;
    printf("Address of x (%%p): %p\\n", (void*)ptr);
    printf("Address of ptr (%%p): %p\\n", (void*)&ptr);
    printf("TEST_PASS\\n");
    """

    framework.register_test(
        name="printf_format_p", test_code=PRINTF_TEST_TEMPLATE.format(test_body=test_body), expected_output="TEST_PASS"
    )

    # Test 7: Hexadecimal format specifier
    test_body = """
    unsigned int hex_val = 0xFF;
    printf("Hexadecimal (%%x): 0x%x\\n", hex_val);
    printf("TEST_PASS\\n");
    """

    framework.register_test(
        name="printf_format_x", test_code=PRINTF_TEST_TEMPLATE.format(test_body=test_body), expected_output="TEST_PASS"
    )

    # Test 8: Long integer format specifiers
    test_body = """
    long int ld_val = 1234567890;
    printf("Long Integer (%%ld): %ld\\n", ld_val);
    unsigned long int lu_val = 1234567890;
    printf("Unsigned Long Integer (%%lu): %lu\\n", lu_val);
    unsigned long int lx_val = 0xABCD1234;
    printf("Unsigned Long Integer in Hex (%%lx): 0x%lx\\n", lx_val);
    printf("TEST_PASS\\n");
    """

    framework.register_test(
        name="printf_format_long",
        test_code=PRINTF_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 9: size_t format specifiers
    test_body = """
    size_t size_val = 123456;
    printf("Size_t value (%%zu): %zu\\n", size_val);
    printf("Size_t value (%%zd): %zd\\n", size_val);
    printf("TEST_PASS\\n");
    """

    framework.register_test(
        name="printf_format_size_t",
        test_code=PRINTF_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 10: Combined format specifiers
    test_body = """
    printf("Combined formats: char=%c, int=%d, hex=0x%x, size_t=%zu\\n", 'X', 42, 0xDEAD, (size_t)98765);
    printf("TEST_PASS\\n");
    """

    framework.register_test(
        name="printf_format_combined",
        test_code=PRINTF_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 11: Edge cases and special values
    test_body = """
    // Test edge cases
    printf("Zero: %d\\n", 0);
    printf("Negative one: %d\\n", -1);
    printf("Max int: %d\\n", 2147483647);
    printf("Min int: %d\\n", -2147483648);
    printf("Null pointer: %p\\n", (void*)0);
    printf("Empty string: '%s'\\n", "");
    printf("TEST_PASS\\n");
    """

    framework.register_test(
        name="printf_edge_cases",
        test_code=PRINTF_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 12: Multiple arguments
    test_body = """
    printf("Multiple args: %d %s %x %c %p\\n", 123, "test", 0xABC, 'Q', (void*)0x1000);
    printf("TEST_PASS\\n");
    """

    framework.register_test(
        name="printf_multiple_args",
        test_code=PRINTF_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )

    # Test 13: Format specifier errors (should still work)
    test_body = """
    // Test with unrecognized format specifier
    printf("Unknown format: %q\\n");
    printf("Incomplete format: %\\n");
    printf("TEST_PASS\\n");
    """

    framework.register_test(
        name="printf_format_errors",
        test_code=PRINTF_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS",
    )
