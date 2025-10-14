#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

/* Include architecture-specific header for syscall definitions */
#include "../../arch/i386/include/syscall.h"

/**
 * Initialize the system call interface.
 *
 * Sets up interrupt 0x80 as the system call entry point, accessible from Ring 3.
 */
void syscall_init(void);

#endif
