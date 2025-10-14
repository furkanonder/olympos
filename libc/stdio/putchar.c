#include <stdio.h>

#if defined(__is_libk)
#include <kernel/tty.h>
#else
#include <unistd.h>
#endif

#ifdef TEST
#include <kernel/serial.h>
#endif

/**
 * Writes a character to the standard output
 *
 * This implementation has multiple modes:
 * - Kernel mode (__is_libk):   Writes directly to the terminal
 * - Test mode (TEST):          Also writes to serial port for test output
 * - User mode:                 Uses system calls to write to console
 *
 * @param ic    The character to write (as an int)
 * @return      The character written as an unsigned char cast to an int, or EOF on error
 */
int putchar(int ic) {
#if defined(__is_libk)
    char c = (char) ic;
    terminal_write(&c, sizeof(c));
#ifdef TEST
    serial_write_char(SERIAL_COM1_BASE, c);
#endif
#else
    char c = (char) ic;
    ssize_t result = write(STDOUT_FILENO, &c, 1);
    if (result != 1) {
        return EOF;
    }
#endif
    return ic;
}
