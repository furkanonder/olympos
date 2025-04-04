#include <string.h>

/**
 * Compare two strings lexicographically
 *
 * @param s1 Pointer to the first string to be compared
 * @param s2 Pointer to the second string to be compared
 * @return   Integer result of comparison:
 *           - 0 if s1 and s2 are equal
 *           - Negative value if s1 is less than s2
 *           - Positive value if s1 is greater than s2
 */
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}