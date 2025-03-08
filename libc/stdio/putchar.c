#include <stdio.h>

#if defined(__is_libk)
#include <kernel/tty.h>
#endif

/**
 * Writes a character to the standard output
 *
 * This implementation has two modes:
 * - Kernel mode (__is_libk):   Writes directly to the terminal
 * - User mode:                 Not yet implemented, will use system calls
 *
 * @param ic 	The character to write (as an int)
 * @return 		The character written as an unsigned char cast to an int
 */
int putchar(int ic) {
#if defined(__is_libk)
    char c = (char) ic;
    terminal_write(&c, sizeof(c));
#else
    // TODO: Implement stdio and the write system call.
#endif
    return ic;
}
