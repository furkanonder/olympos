#include <string.h>

/**
* Copies a memory region to another (possibly overlapping) region
*
* Copies 'size' bytes from memory area 'srcptr' to memory area 'dstptr'.
* The memory regions may overlap. The function ensures correct copying
* by choosing the appropriate copy direction based on relative positions
* of source and destination.
*
* @param dstptr Pointer to destination memory region
* @param srcptr Pointer to source memory region
* @param size   Number of bytes to copy
*
* @return Pointer to the destination memory region (dstptr)
*/
void* memmove(void* dstptr, const void* srcptr, size_t size) {
    unsigned char* dst = (unsigned char*) dstptr;
    const unsigned char* src = (const unsigned char*) srcptr;
    if (dst < src) {
        for (size_t i = 0; i < size; i++) {
            dst[i] = src[i];
        }
    }
    else {
        for (size_t i = size; i != 0; i--) {
            dst[i - 1] = src[i - 1];
        }
    }
    return dstptr;
}
