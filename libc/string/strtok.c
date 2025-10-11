#include <stddef.h>
#include <string.h>

/**
 * Extract tokens from string
 *
 * On first call, pass the string to tokenize. On subsequent calls, pass NULL
 * to continue tokenizing the same string. The function maintains state using
 * a static variable.
 *
 * @param str    String to tokenize (or NULL to continue)
 * @param delim  String containing delimiter characters
 * @return       Pointer to next token, or NULL if no more tokens
 */
char* strtok(char* str, const char* delim) {
    static char* last = NULL;
    
    /* Continue from last position if str is NULL */
    if (str == NULL) {
        str = last;
    }
    /* No string to work with */
    if (str == NULL) {
        return NULL;
    }
    /* Skip leading delimiters */
    str += strspn(str, delim);
    /* End of string? */
    if (*str == '\0') {
        last = NULL;
        return NULL;
    }
    /* Find end of token */
    char* token = str;
    str += strcspn(str, delim);
    /* Null-terminate and save position */
    if (*str != '\0') {
        *str = '\0';
        last = str + 1;
    }
	else {
        last = NULL;
    }
    
    return token;
}
