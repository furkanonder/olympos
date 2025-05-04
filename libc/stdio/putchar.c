#include <stdio.h>

#if defined(__is_libk)
#include <kernel/tty.h>
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
 * - User mode:                 Not yet implemented, will use system calls
 *
 * @param ic 	The character to write (as an int)
 * @return 		The character written as an unsigned char cast to an int
 */
int putchar(int ic) {
#if defined(__is_libk)
    char c = (char) ic;
    terminal_write(&c, sizeof(c));
#ifdef TEST
    serial_write_char(SERIAL_COM1_BASE, c);
#endif
#else
    // TODO: Implement stdio and the write system call.
#endif
    return ic;
}
