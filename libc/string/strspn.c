#include <string.h>

/**
 * Get length of prefix substring
 *
 * Calculates the length of the initial segment of str that consists entirely of characters from accept.
 *
 * @param str     String to be scanned
 * @param accept  String containing characters to match
 * @return        Length of the initial segment of str containing only characters from accept
 */
size_t strspn(const char* str, const char* accept) {
    const char* s = str;
    while (*s && strchr(accept, *s)) {
        s++;
    }
    return s - str;
}

