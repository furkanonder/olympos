#include <stdio.h>

/**
 * Writes a string followed by a newline to stdout
 *
 * Calls printf internally to write the string and append a newline character.
 *
 * @param string    The null-terminated string to write
 * @return          A non-negative number on success, or -1 on error
 */
int puts(const char* string) {
    return printf("%s\n", string);
}
