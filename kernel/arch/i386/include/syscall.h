#ifndef ARCH_I386_SYSCALL_H
#define ARCH_I386_SYSCALL_H

/* System call numbers
 * 
 * These syscall numbers match the Linux i386 syscall table for compatibility.
 * We use the same calling convention: int 0x80 with syscall number in EAX.
 * 
 * @see https://chromium.googlesource.com/chromiumos/docs/+/master/constants/syscalls.md#x86-32_bit
 */
#define SYSCALL_EXIT    1   /* Exit process */
#define SYSCALL_READ    3   /* Read from keyboard */
#define SYSCALL_WRITE   4   /* Write to console */

/**
 * Initialize the system call interface.
 * 
 * Sets up interrupt 0x80 as the system call entry point, accessible from Ring 3.
 */
void syscall_init(void);

#endif
