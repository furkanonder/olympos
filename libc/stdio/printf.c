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
 * - %c:  Character
 * - %s:  String
 * - %d:  Signed integer
 * - %u:  Unsigned integer (decimal)
 * - %p:  Pointer (prefixed with 0x)
 * - %x:  Unsigned integer in hexadecimal
 * - %ld: Long signed integer
 * - %lu: Long unsigned integer
 * - %lx: Long unsigned integer in hexadecimal
 * - %zu: Size_t as unsigned
 * - %zd: Size_t as signed decimal
 *
 * @param format    Format string containing text and format specifiers
 * @param ...       Variable arguments corresponding to format specifiers
 * @return          Number of characters printed, or -1 on error
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
        /* Handle %u format specifier (unsigned integer) */
        else if (*format == 'u') {
            format++;
            /* Get the unsigned integer argument */
            unsigned int u = va_arg(parameters, unsigned int);

            /* Convert unsigned integer to string */
            char num_str[32];
            itoa(u, num_str, 10);

            /* Calculate string length */
            size_t len = strlen(num_str);

            /* Check for potential integer overflow */
            if (maxrem < len) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }

            /* Print the unsigned integer */
            if (!print(num_str, len)) {
                return -1;
            }
            written += len;
        }
        /* Handle %p format specifier (pointer) */
        else if (*format == 'p') {
            format++;
            /* Get the pointer argument */
            void *ptr = va_arg(parameters, void*);

            /* Print "0x" prefix */
            if (maxrem < 2) {
                // TODO: Set errno to EOVERFLOW
                return -1;
            }
            if (!print("0x", 2)) {
                return -1;
            }
            written += 2;
            maxrem -= 2;

            /* Convert pointer to hexadecimal string */
            char ptr_str[32];
            /* Cast to unsigned long to ensure proper size */
            itoa((unsigned long) ptr, ptr_str, 16);

            size_t len = strlen(ptr_str);

            /* Check for potential integer overflow */
            if (maxrem < len) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }

            /* Print the pointer address */
            if (!print(ptr_str, len)) {
                return -1;
            }
            written += len;
        }
        /* Handle %x format specifier (hexadecimal) */
        else if (*format == 'x') {
            format++;
            /* Get the unsigned integer argument */
            unsigned int x = va_arg(parameters, unsigned int);

            /* Convert to hexadecimal string */
            char hex_str[32];
            itoa(x, hex_str, 16);

            /* Calculate string length */
            size_t len = strlen(hex_str);

            /* Check for potential integer overflow */
            if (maxrem < len) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }

            /* Print the hexadecimal */
            if (!print(hex_str, len)) {
                return -1;
            }
            written += len;
        }
        /* Handle long formats: %ld, %lu, and %lx */
        else if (*format == 'l' && (*(format + 1) == 'd' || *(format + 1) == 'u' || *(format + 1) == 'x')) {
            char num_str[32];
            int base = 10;  /* Default base for decimal */

            /* Get format type */
            char type = *(format + 1);
            format += 2;  /* Skip both 'l' and the type char */

            /* Hexadecimal unsigned long */
            if (type == 'x') {
                base = 16;
                unsigned long ul = va_arg(parameters, unsigned long);
                itoa(ul, num_str, base);
            }
            /* Signed long */
            else if (type == 'd') {
                long l = va_arg(parameters, long);
                itoa(l, num_str, base);
            }
            /* Unsigned long */
            else {
                unsigned long ul = va_arg(parameters, unsigned long);
                itoa(ul, num_str, base);
            }

            /* Calculate string length */
            size_t len = strlen(num_str);

            /* Check for potential integer overflow */
            if (maxrem < len) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }

            /* Print the number */
            if (!print(num_str, len)) {
                return -1;
            }
            written += len;
        }
        /* Handle %z format specifier (size_t) */
        else if (*format == 'z' && (*(format + 1) == 'u' || *(format + 1) == 'd')) {
            char num_str[32];
            /* Skip both 'z' and the type char */
            format += 2;

            /* Get the size_t argument */
            size_t z = va_arg(parameters, size_t);

            /* Convert to string */
            itoa(z, num_str, 10);

            /* Calculate string length */
            size_t len = strlen(num_str);

            /* Check for potential integer overflow */
            if (maxrem < len) {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }

            /* Print the number */
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
