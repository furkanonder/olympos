#include <stdio.h>
#include <stdlib.h>

/**
* Abnormally terminates the program
*
* This function has different behaviors in kernel and user mode:
* - Kernel mode (__is_libk):    Triggers a kernel panic
* - User mode:                  Will terminate process with SIGABRT (not yet implemented)
*
* @note This function never returns
* @note Currently just prints a message and enters infinite loop
*/
__attribute__((__noreturn__))
void abort(void) {
#if defined(__is_libk)
    // TODO: Add proper kernel panic.
    printf("kernel: panic: abort()\n");
#else
    // TODO: Abnormally terminate the process as if by SIGABRT.
    printf("abort()\n");
#endif
    while (1) { }
    __builtin_unreachable();
}
