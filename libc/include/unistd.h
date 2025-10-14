/**
 * unistd.h - Standard symbolic constants and types (POSIX)
 * 
 * This header provides access to the POSIX operating system API, including
 * system calls for I/O, process control, and file operations.
 * 
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/unistd.h.html
 */

#ifndef _UNISTD_H
#define _UNISTD_H 1

#include <sys/syscall.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Standard file descriptors */
#define STDIN_FILENO    0   /* Standard input file descriptor */
#define STDOUT_FILENO   1   /* Standard output file descriptor */
#define STDERR_FILENO   2   /* Standard error file descriptor */

/**
 * Write data to a file descriptor.
 * 
 * @param fd File descriptor
 * @param buf Buffer containing data to write
 * @param count Number of bytes to write
 * @return Number of bytes written on success, -1 on error
 */
ssize_t write(int fd, const void *buf, size_t count);

/**
 * Read data from a file descriptor.
 * 
 * @param fd File descriptor
 * @param buf Buffer to store read data
 * @param count Maximum number of bytes to read
 * @return Number of bytes read on success, -1 on error, 0 on EOF
 */
ssize_t read(int fd, void *buf, size_t count);

/**
 * Terminate the calling process.
 * 
 * @param status Exit status code (0 for success, non-zero for error)
 */
void _exit(int status) __attribute__((noreturn));

/**
 * Generic system call interface (Linux-compatible).
 * 
 * Makes a raw system call with the specified syscall number and up to 5 arguments.
 * This is a low-level interface - prefer using the specific wrapper functions
 * (write, read, etc.) when available.
 * 
 * @param number Syscall number (e.g., SYS_write, SYS_read)
 * @param ... Up to 5 arguments for the syscall
 * @return Return value from the syscall (meaning depends on the specific syscall)
 * 
 * @example
 * syscall(SYS_write, 1, "Hello\n", 6);
 * syscall(SYS_exit, 0);
 */
long syscall(long number, ...);

#ifdef __cplusplus
}
#endif

#endif /* _UNISTD_H */
