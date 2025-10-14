/**
 * Read character from standard input
 *
 * Reads a single character from standard input. In kernel mode, this interfaces with the
 * keyboard driver to provide blocking character input. In user mode, uses system calls.
 *
 * @return  The character read as an int, or EOF on end-of-file
 */

#include <stdio.h>

#if defined(__is_libk)
#include <kernel/keyboard.h>
#else
#include <unistd.h>
#endif

int getchar(void) {
#if defined(__is_libk)
    return keyboard_callback_getchar();
#else
    char c;
    ssize_t result = read(STDIN_FILENO, &c, 1);
    if (result == 1) {
        return (unsigned char) c;
    }
    else {
        return EOF;
    }
#endif
}
