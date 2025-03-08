#include <string.h>

/**
 * Calculates the length of a null-terminated string
 *
 * Counts characters until a null terminator ('\0') is found.
 * The null terminator is not included in the returned length.
 *
 * @param str Pointer to the null-terminated string
 * @return Number of characters in the string before the null terminator
 */
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}
