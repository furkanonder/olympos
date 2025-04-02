#include <stdio.h>

__attribute__((__noreturn__))
void __assert_fail(const char *expr, const char *file, unsigned int line, const char *function) {
    printf("%s: %s:%d: %s: Assertion `%s' failed.\n", "kernel", file, line, function, expr);
    while (1) {
        asm volatile ("hlt");
    }
    __builtin_unreachable();
}
