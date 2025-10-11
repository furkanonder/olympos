#include <string.h>

/**
 * Get length of prefix substring not containing reject characters
 *
 * Calculates the length of the initial segment of str that consists entirely of characters not in reject.
 *
 * @param str     String to be scanned
 * @param reject  String containing characters to avoid
 * @return        Length of the initial segment of str containing no characters from reject
 */
size_t strcspn(const char* str, const char* reject) {
    const char* s = str;
    while (*s && !strchr(reject, *s)) {
        s++;
    }
    return s - str;
}
