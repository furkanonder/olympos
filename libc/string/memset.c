#include <string.h>

/**
* Fills a memory region with a specified byte value
*
* Sets 'size' bytes of memory starting at 'bufptr' to the specified value.
* The value is truncated to an unsigned char.
*
* @param bufptr Pointer to the memory region to fill
* @param value  Value to write (converted to unsigned char)
* @param size   Number of bytes to fill
*
* @return Pointer to the filled memory region (bufptr)
*/
void* memset(void* bufptr, int value, size_t size) {
    unsigned char* buf = (unsigned char*) bufptr;
    for (size_t i = 0; i < size; i++) {
        buf[i] = (unsigned char) value;
    }
    return bufptr;
}
