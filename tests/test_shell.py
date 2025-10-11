from test_framework import OlymposTestFramework

SHELL_TEST_TEMPLATE = """
#include <kernel/serial.h>
#include <kernel/kheap.h>
#include <kernel/shell.h>

// Exit QEMU function
void exit_qemu(uint32_t exit_code) {{
    asm volatile("outl %0, %1" : : "a"(exit_code), "Nd"((uint16_t)0xf4));
}}

void kernel_main(unsigned long magic, unsigned long addr) {{
    paging_init();
    kheap_init();
    terminal_initialize();
    serial_setup(SERIAL_COM1_BASE, SERIAL_BAUD_115200);
    
    serial_write_string(SERIAL_COM1_BASE, "TEST_RUNNING\\n");
        
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


def register_shell_tests(framework: OlymposTestFramework):
    # Test 1: Built-in command count
    test_body = """
    int count = shell_num_builtins();
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Built-in count: %d\\n", count);
    serial_write_string(SERIAL_COM1_BASE, buffer);
    
    if (count == 2) {
        serial_write_string(SERIAL_COM1_BASE, "TEST_PASS\\n");
    }
    else {
        serial_write_string(SERIAL_COM1_BASE, "TEST_FAIL\\n");
    }
    """

    framework.register_test(
        name="shell_builtin_count",
        test_code=SHELL_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS"
    )

    # Test 2: Parse empty line
    test_body = """   
    char line[] = "";
    char** tokens = parse_line(line);
    
    if (tokens != NULL && tokens[0] == NULL) {
        serial_write_string(SERIAL_COM1_BASE, "Empty line parsed correctly\\n");
        serial_write_string(SERIAL_COM1_BASE, "TEST_PASS\\n");
    }
    else {
        serial_write_string(SERIAL_COM1_BASE, "TEST_FAIL\\n");
    }
    """

    framework.register_test(
        name="shell_parse_empty_line",
        test_code=SHELL_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS"
    )

    # Test 3: Parse command with arguments
    test_body = """
    char line[] = "test hello world";
    char** tokens = parse_line(line);
    
    int pass = 0;
    if (tokens != NULL && tokens[0] != NULL) {
        if (strcmp(tokens[0], "test") == 0 && tokens[1] != NULL && strcmp(tokens[1], "hello") == 0 &&
            tokens[2] != NULL && strcmp(tokens[2], "world") == 0 && tokens[3] == NULL) {
            serial_write_string(SERIAL_COM1_BASE, "Command with arguments parsed correctly\\n");
            pass = 1;
        }
    }
    
    if (pass) {
        serial_write_string(SERIAL_COM1_BASE, "TEST_PASS\\n");
    }
    else {
        serial_write_string(SERIAL_COM1_BASE, "TEST_FAIL\\n");
    }
    """

    framework.register_test(
        name="shell_parse_command_with_args",
        test_code=SHELL_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS"
    )

    # Test 4: Parse command with multiple spaces
    test_body = """  
    char line[] = "  help   ";
    char** tokens = parse_line(line);
    
    int pass = 0;
    if (tokens != NULL && tokens[0] != NULL) {
        if (strcmp(tokens[0], "help") == 0 && tokens[1] == NULL) {
            serial_write_string(SERIAL_COM1_BASE, "Command with extra spaces parsed correctly\\n");
            pass = 1;
        }
    }
    
    if (pass) {
        serial_write_string(SERIAL_COM1_BASE, "TEST_PASS\\n");
    }
    else {
        serial_write_string(SERIAL_COM1_BASE, "TEST_FAIL\\n");
    }
    """

    framework.register_test(
        name="shell_parse_extra_spaces",
        test_code=SHELL_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS"
    )

    # Test 5: Execute valid help command
    test_body = """
    char line[] = "help";
    char** tokens = parse_line(line);
    
    int result = shell_execute(tokens);
    
    if (result == 1) {
        serial_write_string(SERIAL_COM1_BASE, "Help command executed successfully\\n");
        serial_write_string(SERIAL_COM1_BASE, "TEST_PASS\\n");
    }
    else {
        serial_write_string(SERIAL_COM1_BASE, "TEST_FAIL\\n");
    }
    """

    framework.register_test(
        name="shell_execute_help",
        test_code=SHELL_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS"
    )

    # Test 6: Execute valid clear command
    test_body = """
    char line[] = "clear";
    char** tokens = parse_line(line);
    
    int result = shell_execute(tokens);
    
    if (result == 1) {
        serial_write_string(SERIAL_COM1_BASE, "Clear command executed successfully\\n");
        serial_write_string(SERIAL_COM1_BASE, "TEST_PASS\\n");
    }
    else {
        serial_write_string(SERIAL_COM1_BASE, "TEST_FAIL\\n");
    }
    """

    framework.register_test(
        name="shell_execute_clear",
        test_code=SHELL_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS"
    )

    # Test 7: Execute unknown command
    test_body = """
    char line[] = "unknowncommand";
    char** tokens = parse_line(line);
    
    int result = shell_execute(tokens);
    
    if (result == 1) {
        serial_write_string(SERIAL_COM1_BASE, "Unknown command handled correctly\\n");
        serial_write_string(SERIAL_COM1_BASE, "TEST_PASS\\n");
    }
    else {
        serial_write_string(SERIAL_COM1_BASE, "TEST_FAIL\\n");
    }
    """

    framework.register_test(
        name="shell_execute_unknown",
        test_code=SHELL_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS"
    )

    # Test 8: Execute empty command
    test_body = """
    char line[] = "";
    char** tokens = parse_line(line);
    
    int result = shell_execute(tokens);
    
    if (result == 1) {
        serial_write_string(SERIAL_COM1_BASE, "Empty command handled correctly\\n");
        serial_write_string(SERIAL_COM1_BASE, "TEST_PASS\\n");
    }
    else {
        serial_write_string(SERIAL_COM1_BASE, "TEST_FAIL\\n");
    }
    """

    framework.register_test(
        name="shell_execute_empty",
        test_code=SHELL_TEST_TEMPLATE.format(test_body=test_body),
        expected_output="TEST_PASS"
    )
