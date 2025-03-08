#include <string.h>

/**
* Compares two memory regions byte by byte
*
* Compares the first 'size' bytes of memory regions pointed to by 'aptr' and 'bptr'.
* The comparison is done treating data as unsigned chars.
*
* @param aptr Pointer to first memory region
* @param bptr Pointer to second memory region
* @param size Number of bytes to compare
*
* @return  0 if memory regions are identical
*         -1 if first differing byte in aptr is less than bptr
*          1 if first differing byte in aptr is greater than bptr
*/
int memcmp(const void* aptr, const void* bptr, size_t size) {
    const unsigned char* a = (const unsigned char*) aptr;
    const unsigned char* b = (const unsigned char*) bptr;
    for (size_t i = 0; i < size; i++) {
        if (a[i] < b[i]) {
            return -1;
        }
        else if (b[i] < a[i]) {
            return 1;
        }
    }
    return 0;
}
