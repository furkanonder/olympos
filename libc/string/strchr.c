#include <stddef.h>

/**
 * Locate character in string
 *
 * Searches for the first occurrence of character c in the string pointed to by str. The null terminator is considered
 * part of the string, so searching for '\0' will return a pointer to the null terminator.
 *
 * @param str  Pointer to the string to be searched
 * @param c    Character to search for (passed as int, but converted to char)
 * @return     Pointer to the first occurrence of c in str, or NULL if not found
 */
char* strchr(const char* str, int c) {
    // Loop through each character in the string
    while (*str != '\0') {
        if (*str == (char) c) {
            return (char *) str;	// Return a pointer to the character
        }
        str++;  					// Move to the next character
    }
    // If character wasn't found, check if c is the null terminator
    if (c == '\0') {
        return (char *) str;		// Return pointer to the null terminator
    }
    return NULL; 			 		// Character was not found
}
