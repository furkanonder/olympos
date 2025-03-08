#include <string.h>

/**
* Copies a memory region to another non-overlapping region
*
* Copies 'size' bytes from memory area 'srcptr' to memory area 'dstptr'.
* The regions must not overlap. Use memmove for potentially overlapping regions.
*
* @param dstptr Pointer to destination memory region (restrict)
* @param srcptr Pointer to source memory region (restrict)
* @param size   Number of bytes to copy
*
* @return Pointer to the destination memory region (dstptr)
*/
void* memcpy(void* restrict dstptr, const void* restrict srcptr, size_t size) {
    unsigned char* dst = (unsigned char*) dstptr;
    const unsigned char* src = (const unsigned char*) srcptr;
    for (size_t i = 0; i < size; i++) {
        dst[i] = src[i];
    }
    return dstptr;
}
