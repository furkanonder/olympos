#ifndef _SYS_SYSCALL_H
#define _SYS_SYSCALL_H 1

#include <stddef.h>   /* For size_t */
#include <stdint.h>   /* For intptr_t */

/* Define ssize_t if not already defined */
#ifndef _SSIZE_T_DEFINED
#define _SSIZE_T_DEFINED
typedef long ssize_t;
#endif

/* System call numbers - Linux i386 compatible
 * 
 * Must match kernel/include/kernel/syscall.h
 * These numbers align with Linux i386 syscall table for compatibility.
 */
#define SYS_EXIT    1   /* Exit process (matches Linux sys_exit) */
#define SYS_READ    3   /* Read from keyboard (matches Linux sys_read) */
#define SYS_WRITE   4   /* Write to console (matches Linux sys_write) */

/* Linux-style lowercase aliases for compatibility */
#define SYS_exit    SYS_EXIT
#define SYS_read    SYS_READ
#define SYS_write   SYS_WRITE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * System call wrappers for user space programs. 
 * These functions use 'int 0x80' to invoke kernel system calls.
 * Full documentation is in unistd.h
 */
ssize_t write(int fd, const void *buf, size_t count);
ssize_t read(int fd, void *buf, size_t count);
void exit(int status) __attribute__((noreturn));
void _exit(int status) __attribute__((noreturn));

/**
 * Generic system call interface (Linux-compatible).
 * 
 * Makes a raw system call with the specified syscall number and up to 5 arguments.
 * 
 * @param number Syscall number (e.g., SYS_write, SYS_read)
 * @param ... Up to 5 arguments for the syscall
 * @return Return value from the syscall
 */
long syscall(long number, ...);

#ifdef __cplusplus
}
#endif

#endif
