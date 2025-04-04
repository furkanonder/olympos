#ifndef _ASSERT_H
#define _ASSERT_H 1

#include <stdio.h>

__attribute__((noreturn))
void __assert_fail(const char *expr, const char *file, unsigned int line, const char *function);

#define assert(expr) ((expr) ? ((void)0) : __assert_fail(#expr, __FILE__, __LINE__, __func__))

#define panic(fmt, ...) do {                                \
    asm volatile("cli");                                    \
    printf("Kernel panic: " fmt "\n", ##__VA_ARGS__);       \
    print_backtrace();                                      \
    while (1) {                                             \
        asm volatile("hlt");                                \
    }                                                       \
    __builtin_unreachable();                                \
} while (0)

#endif
