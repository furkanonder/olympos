#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Helper function to print a sequence of bytes
 *
 * @param data 		Pointer to the data to print
 * @param length 	Number of bytes to print
 * @return 			true if successful, false if a write error occurred
 */
static bool print(const char* data, size_t length) {
    const unsigned char* bytes = (const unsigned char*) data;
    for (size_t i = 0; i < length; i++) {
        if (putchar(bytes[i]) == EOF) {
            return false;
        }
    }
    return true;
}

/**
 * Writes formatted output to stdout
 *
 * Supports the following format specifiers:
 * - %c: Character
 * - %s: String
 *
 * @param format 	Format string containing text and format specifiers
 * @param ... 		Variable arguments corresponding to format specifiers
 * @return 			Number of characters printed, or -1 on error
 */
int printf(const char* restrict format, ...) {
    va_list parameters;
    va_start(parameters, format);  /* Initialize variable argument list */

    int written = 0;  /* Track number of characters written */

    /* Process the format string character by character */
    while (*format != '\0') {
        /* Calculate remaining space before integer overflow */
        size_t maxrem = INT_MAX - written;

        /* Handle regular characters and %% escape sequences */
        if (format[0] != '%' || format[1] == '%') {
            /* Skip one character for %% to print a single % */
            if (format[0] == '%') {
                format++;
            }

            /* Count consecutive non-format characters */
            size_t amount = 1;
            while (format[amount] && format[amount] != '%') {
                amount++;
            }

            /* Check for potential integer overflow */
            if (maxrem < amount) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }

            /* Print the character sequence */
            if (!print(format, amount)) {
                return -1;
            }

            /* Update position and count */
            format += amount;
            written += amount;
            continue;
        }

        /* Save the position of the format specifier */
        const char* format_begun_at = format++;

        /* Handle %c format specifier (character) */
        if (*format == 'c') {
            format++;
            /* Get the character argument (note: char is promoted to int in varargs) */
            char c = (char) va_arg(parameters, int);

            /* Check for potential integer overflow */
            if (!maxrem) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }

            /* Print the character */
            if (!print(&c, sizeof(c))) {
                return -1;
            }
            written++;
        }
        /* Handle %s format specifier (string) */
        else if (*format == 's') {
            format++;
            /* Get the string argument */
            const char* str = va_arg(parameters, const char*);
            size_t len = strlen(str);

            /* Check for potential integer overflow */
            if (maxrem < len) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }

            /* Print the string */
            if (!print(str, len)) {
                return -1;
            }
            written += len;
        }
        /* Handle %d format specifier (integer) */
        else if (*format == 'd') {
            format++;
            /* Get the integer argument */
            int i = va_arg(parameters, int);

            /* Convert integer to string */
            char num_str[32];
            itoa(i, num_str, 10);

            /* Calculate string lenght */
            size_t len = strlen(num_str);

            /* Check for potential integer overflow */
            if (maxrem < len) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }

            /* Print the integer */
            if (!print(num_str, len)) {
                return -1;
            }
            written += len;
        }
        /* Handle unrecognized format specifier */
        else {
            /* Reset to the beginning of the format specifier */
            format = format_begun_at;
            size_t len = strlen(format);

            /* Check for potential integer overflow */
            if (maxrem < len) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }

            /* Print the format specifier as literal text */
            if (!print(format, len)) {
                return -1;
            }
            written += len;
            format += len;
        }
    }

    /* Clean up variable argument list */
    va_end(parameters);
    return written;
}
