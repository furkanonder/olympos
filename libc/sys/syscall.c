#include <sys/syscall.h>

/**
 * System call wrapper implementations for user space.
 * 
 * These functions use inline assembly to trigger the system call interrupt (int 0x80). The kernel system call handler
 * reads the syscall number from EAX and arguments from other registers (EBX, ECX, EDX, etc.).
 * 
 * Linux i386 syscall convention:
 * - EAX: syscall number (input) / return value (output)
 * - EBX: arg1 (e.g., file descriptor)
 * - ECX: arg2 (e.g., buffer pointer)
 * - EDX: arg3 (e.g., count)
 * - ESI: arg4
 * - EDI: arg5
 * 
 * GCC Inline Assembly Syntax (Extended ASM):
 * asm volatile (
 *     "assembly code"              // Instructions to execute
 *     : "output constraints"       // How to map C variables to output registers
 *     : "input constraints"        // How to map C variables to input registers
 *     : "clobber list"            // Registers/memory modified by the assembly
 * );
 * 
 * Common Constraint Codes for x86:
 * - "a" = EAX register    - "b" = EBX register    - "c" = ECX register    - "d" = EDX register
 * - "S" = ESI register    - "D" = EDI register
 * - "r" = any general purpose register (let compiler choose)
 * - "m" = memory operand  - "i" = immediate constant
 * - "=" prefix = write-only (output)
 * - "+" prefix = read-write (input and output)
 * - No prefix = read-only (input)
 * 
 * @see https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html
 * @see https://wiki.osdev.org/Inline_Assembly/Examples
 */

/**
 * Write data to a file descriptor.
 * 
 * Inline Assembly Breakdown:
 * - "int $0x80"        : Trigger software interrupt 0x80 (syscall entry point)
 * - "=a" (ret)         : Output - store EAX register value into 'ret' after syscall
 * - "a" (4)            : Input - load 4 (SYS_WRITE) into EAX register
 * - "b" (fd)           : Input - load 'fd' into EBX register (arg1)
 * - "c" (buf)          : Input - load 'buf' pointer into ECX register (arg2)
 * - "d" (count)        : Input - load 'count' into EDX register (arg3)
 * - "memory"           : Clobber - tell compiler memory might change (syscall side effects)
 */
ssize_t write(int fd, const void *buf, size_t count) {
    ssize_t ret;
    asm volatile (
        "int $0x80"                 /* Trigger syscall interrupt */
        : "=a" (ret)                /* Output: EAX → ret */
        : "a" (4), "b" (fd), "c" (buf), "d" (count)  /* Inputs: EAX=4, EBX=fd, ECX=buf, EDX=count */
        : "memory"                  /* Clobbers: memory may be modified */
    );
    return ret;
}

/**
 * Read data from a file descriptor.
 * 
 * Inline Assembly Breakdown:
 * - "int $0x80"        : Trigger software interrupt 0x80 (syscall entry point)
 * - "=a" (ret)         : Output - store EAX register value into 'ret' after syscall
 * - "a" (3)            : Input - load 3 (SYS_READ) into EAX register
 * - "b" (fd)           : Input - load 'fd' into EBX register (arg1)
 * - "c" (buf)          : Input - load 'buf' pointer into ECX register (arg2)
 * - "d" (count)        : Input - load 'count' into EDX register (arg3)
 * - "memory"           : Clobber - tell compiler memory might change (buffer will be filled)
 */
ssize_t read(int fd, void *buf, size_t count) {
    ssize_t ret;
    asm volatile (
        "int $0x80"                 /* Trigger syscall interrupt */
        : "=a" (ret)                /* Output: EAX → ret (bytes read) */
        : "a" (3), "b" (fd), "c" (buf), "d" (count)  /* Inputs: EAX=3, EBX=fd, ECX=buf, EDX=count */
        : "memory"                  /* Clobbers: buffer contents will be modified */
    );
    return ret;
}

/**
 * Exit the current user mode program.
 * 
 * Inline Assembly Breakdown:
 * - "int $0x80"        : Trigger software interrupt 0x80 (syscall entry point)
 * - No outputs         : This syscall never returns (program exits)
 * - "a" (1)            : Input - load 1 (SYS_EXIT) into EAX register
 * - "b" (status)       : Input - load 'status' into EBX register (exit code)
 * - No clobbers needed : Function never returns, so compiler state doesn't matter
 */
void exit(int status) {
    asm volatile (
        "int $0x80"                 /* Trigger syscall interrupt - never returns */
        :                           /* No outputs - this syscall terminates the process */
        : "a" (1), "b" (status)     /* Inputs: EAX=1 (SYS_EXIT), EBX=status */
    );
    /* Should never reach here - kernel halts the process */
    while (1);
}

/**
 * Alternate name for exit (POSIX _exit).
 */
void _exit(int status) {
    exit(status);
}

/**
 * Generic system call interface.
 * 
 * Makes a raw system call with up to 5 arguments. This follows the Linux i386 syscall convention using int 0x80.
 * 
 * Inline Assembly Breakdown:
 * - "int $0x80"        : Trigger software interrupt 0x80 (syscall entry point)
 * - "=a" (ret)         : Output - store EAX register value into 'ret' after syscall
 * - "a" (number)       : Input - load syscall number into EAX register
 * - "b" (arg1)         : Input - load arg1 into EBX register
 * - "c" (arg2)         : Input - load arg2 into ECX register
 * - "d" (arg3)         : Input - load arg3 into EDX register
 * - "S" (arg4)         : Input - load arg4 into ESI register
 * - "D" (arg5)         : Input - load arg5 into EDI register
 * - "memory"           : Clobber - tell compiler memory might change
 * 
 * GCC Register Constraint Codes:
 * - "a" = EAX/AL     - "b" = EBX/BL     - "c" = ECX/CL     - "d" = EDX/DL
 * - "S" = ESI/SI     - "D" = EDI/DI
 * - "=" prefix means write-only output
 * - No "=" means read-only input
 * 
 * Note: For simplicity, we pass all register values. Unused arguments are initialized
 * to 0 and ignored by the kernel. This matches Linux syscall behavior.
 */
long syscall(long number, ...) {
    long ret;
    long arg1 = 0, arg2 = 0, arg3 = 0, arg4 = 0, arg5 = 0;
    
    /* Extract variable arguments using GCC builtins */
    __builtin_va_list args;
    __builtin_va_start(args, number);
    
    /* Get arguments if provided - we assume max 5 args (Linux i386 limit) */
    arg1 = __builtin_va_arg(args, long);
    arg2 = __builtin_va_arg(args, long);
    arg3 = __builtin_va_arg(args, long);
    arg4 = __builtin_va_arg(args, long);
    arg5 = __builtin_va_arg(args, long);
    
    __builtin_va_end(args);
    
    /* Make the system call - all 6 registers set up per Linux i386 ABI */
    asm volatile (
        "int $0x80"                 /* Trigger syscall interrupt */
        : "=a" (ret)                /* Output: EAX → ret (syscall return value) */
        : "a" (number),             /* Input: EAX = syscall number */
          "b" (arg1),               /* Input: EBX = argument 1 */
          "c" (arg2),               /* Input: ECX = argument 2 */
          "d" (arg3),               /* Input: EDX = argument 3 */
          "S" (arg4),               /* Input: ESI = argument 4 */
          "D" (arg5)                /* Input: EDI = argument 5 */
        : "memory"                  /* Clobbers: syscall may modify memory */
    );
    
    return ret;
}
