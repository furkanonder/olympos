#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

/**
 * Helper function to write formatted output to a string buffer
 *
 * @param buffer    Pointer to the buffer to write to
 * @param size      Maximum number of bytes to write (including null terminator)
 * @param format    Format string containing text and format specifiers
 * @param args      Variable arguments corresponding to format specifiers
 * @return          Number of characters that would have been written if size had been sufficient,
 *                  not counting the null terminator, or -1 on error
 */
int vsnprintf(char* restrict buffer, size_t size, const char* restrict format, va_list args) {
    if (size == 0) {
        return 0;
    }
    if (buffer == NULL || format == NULL) {
        return -1;
    }

    size_t buffer_pos = 0;
    int virtual_len = 0;  // Track characters that would be written even if buffer is full

    /* Process the format string character by character */
    while (*format != '\0') {
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

            /* Write to buffer if space available */
            for (size_t i = 0; i < amount; i++) {
                virtual_len++;
                if (buffer_pos + 1 < size) {
                    buffer[buffer_pos++] = format[i];
                }
            }

            format += amount;
            continue;
        }

        /* Save the position of the format specifier */
        const char* format_begun_at = format++;

        /* Handle %c format specifier (character) */
        if (*format == 'c') {
            format++;
            /* Get the character argument (note: char is promoted to int in varargs) */
            char c = (char) va_arg(args, int);

            virtual_len++;
            if (buffer_pos + 1 < size) {
                buffer[buffer_pos++] = c;
            }
        }
        /* Handle %s format specifier (string) */
        else if (*format == 's') {
            format++;
            /* Get the string argument */
            const char* str = va_arg(args, const char*);
            size_t len = strlen(str);

            virtual_len += len;
            /* Copy string to buffer */
            size_t copy_len = (buffer_pos + len < size) ? len : (size - buffer_pos - 1);
            if (copy_len > 0) {
                memcpy(buffer + buffer_pos, str, copy_len);
                buffer_pos += copy_len;
            }
        }
        /* Handle %d format specifier (integer) */
        else if (*format == 'd') {
            format++;
            /* Get the integer argument */
            int i = va_arg(args, int);

            /* Convert integer to string */
            char num_str[32];
            itoa(i, num_str, 10);

            /* Calculate string length */
            size_t len = strlen(num_str);
            virtual_len += len;

            /* Copy integer string to buffer */
            size_t copy_len = (buffer_pos + len < size) ? len : (size - buffer_pos - 1);
            if (copy_len > 0) {
                memcpy(buffer + buffer_pos, num_str, copy_len);
                buffer_pos += copy_len;
            }
        }
        /* Handle %p format specifier (pointer) */
        else if (*format == 'p') {
            format++;
            /* Get the pointer argument */
            void *ptr = va_arg(args, void*);

            /* Convert pointer to hexadecimal string */
            char ptr_str[32];
            itoa((unsigned long) ptr, ptr_str, 16);

            /* Calculate total length with "0x" prefix */
            size_t prefix_len = 2;  // Length of "0x"
            size_t num_len = strlen(ptr_str);
            size_t total_len = prefix_len + num_len;
            virtual_len += total_len;

            /* Copy "0x" prefix and pointer string if space available */
            if (buffer_pos + 1 < size) {
                size_t remaining = size - buffer_pos - 1;

                /* Copy "0x" prefix if space */
                if (remaining >= 1) {
                    buffer[buffer_pos++] = '0';
                    remaining--;
                }
                if (remaining >= 1) {
                    buffer[buffer_pos++] = 'x';
                    remaining--;
                }

                /* Copy pointer digits */
                size_t copy_len = (remaining < num_len) ? remaining : num_len;
                if (copy_len > 0) {
                    memcpy(buffer + buffer_pos, ptr_str, copy_len);
                    buffer_pos += copy_len;
                }
            }
        }
        /* Handle %x format specifier (hexadecimal) */
        else if (*format == 'x') {
            format++;
            /* Get the unsigned integer argument */
            unsigned int x = va_arg(args, unsigned int);

            /* Convert to hexadecimal string */
            char hex_str[32];
            itoa(x, hex_str, 16);

            /* Calculate string length */
            size_t len = strlen(hex_str);
            virtual_len += len;

            /* Copy hex string to buffer */
            size_t copy_len = (buffer_pos + len < size) ? len : (size - buffer_pos - 1);
            if (copy_len > 0) {
                memcpy(buffer + buffer_pos, hex_str, copy_len);
                buffer_pos += copy_len;
            }
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
                unsigned long ul = va_arg(args, unsigned long);
                itoa(ul, num_str, base);
            }
            /* Signed long */
            else if (type == 'd') {
                long l = va_arg(args, long);
                itoa(l, num_str, base);
            }
            /* Unsigned long */
            else {
                unsigned long ul = va_arg(args, unsigned long);
                itoa(ul, num_str, base);
            }

            /* Calculate string length */
            size_t len = strlen(num_str);
            virtual_len += len;

            /* Copy number string to buffer */
            size_t copy_len = (buffer_pos + len < size) ? len : (size - buffer_pos - 1);
            if (copy_len > 0) {
                memcpy(buffer + buffer_pos, num_str, copy_len);
                buffer_pos += copy_len;
            }
        }
        /* Handle %z format specifier (size_t) */
        else if (*format == 'z' && (*(format + 1) == 'u' || *(format + 1) == 'd')) {
            char num_str[32];
            /* Skip both 'z' and the type char */
            format += 2;

            /* Get the size_t argument */
            size_t z = va_arg(args, size_t);

            /* Convert to string */
            itoa(z, num_str, 10);

            /* Calculate string length */
            size_t len = strlen(num_str);
            virtual_len += len;

            /* Copy number string to buffer */
            size_t copy_len = (buffer_pos + len < size) ? len : (size - buffer_pos - 1);
            if (copy_len > 0) {
                memcpy(buffer + buffer_pos, num_str, copy_len);
                buffer_pos += copy_len;
            }
        }
        /* Handle unrecognized format specifier */
        else {
            /* Reset to the beginning of the format specifier */
            format = format_begun_at;
            size_t len = strlen(format);

            virtual_len += len;
            /* Copy the format specifier as literal text */
            size_t copy_len = (buffer_pos + len < size) ? len : (size - buffer_pos - 1);
            if (copy_len > 0) {
                memcpy(buffer + buffer_pos, format, copy_len);
                buffer_pos += copy_len;
            }
            format += len;
        }
    }

    /* Null-terminate the buffer */
    if (size > 0) {
        buffer[buffer_pos] = '\0';
    }

    return virtual_len;
}
