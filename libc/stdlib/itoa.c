/**
 *  Converts an integer value to a null-terminated string using the
 *  specified base and stores the result in the array given by str parameter.
 *
 *  If base is 10 and value is negative, the resulting string is preceded with a minus sign (-).
 *  With any other base, value is always considered unsigned.
 *  Based on: http://www.strudel.org.uk/itoa/
 *
 *  @param value     Value to be converted to a string.
 *  @param str       Array in memory where to store the resulting null-terminated string.
 *  @param base      Numerical base used to represent the value as a string,
 *                   between 2 and 36, where 10 means decimal base,
 *                   16 hexadecimal, 8 octal, and 2 binary.
 *  @return          A pointer to the resulting null-terminated string, same as parameter str.
 */
char* itoa(int value, char* str, int base) {
    // Check that the base if valid
    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }

    char *ptr = str, *ptr1 = str, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while(value);

    // Apply negative sign
    if (tmp_value < 0 && base == 10) {
        *ptr++ = '-';
    }

    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }

    return str;
}
