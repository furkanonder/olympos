/**
 * Read character from standard input
 *
 * Reads a single character from standard input. In kernel mode, this interfaces with the
 * keyboard driver to provide blocking character input.
 *
 * @return  The character read as an int, or EOF on end-of-file
 */

#include <stdio.h>

#if defined(__is_libk)
#include <kernel/keyboard.h>
#endif

int getchar(void) {
#if defined(__is_libk)
    return keyboard_callback_getchar();
#else
    // TODO: Implement stdio and the read system call.
#endif
}
