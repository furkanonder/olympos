#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>

/**
 * Write formatted output to a string buffer
 *
 * @param buffer    Pointer to the buffer to write to
 * @param size      Maximum number of bytes to write (including null terminator)
 * @param format    Format string containing text and format specifiers
 * @param ...       Variable arguments corresponding to format specifiers
 * @return          Number of characters that would have been written if size had been sufficient,
 *                  not counting the null terminator, or -1 on error
 */
int snprintf(char* restrict buffer, size_t size, const char* restrict format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsnprintf(buffer, size, format, args);
    va_end(args);
    return result;
}