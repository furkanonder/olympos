/**
 * System Call Interface (int 0x80)
 *
 * This module implements the system call interface for user-space programs. When a user program needs kernel
 * services (I/O, memory allocation, etc.), it uses the 'int 0x80' instruction to trigger a controlled transition
 * from Ring 3 (user mode) to Ring 0 (kernel mode).
 *
 * System Call Mechanism:
 * 1. User program executes 'int 0x80' with syscall number in EAX
 * 2. CPU triggers interrupt 128 (0x80) via IDT
 * 3. Hardware switches to kernel stack (from TSS)
 * 4. syscall_handler() runs in Ring 0
 * 5. Handler validates request and performs privileged operation
 * 6. iret returns to Ring 3 with result in EAX
 *
 * Register Convention (follows Linux i386 ABI):
 * - EAX: System call number (input) / Return value (output)
 * - EBX: Argument 1 (e.g., file descriptor for read/write)
 * - ECX: Argument 2 (e.g., buffer pointer for read/write)
 * - EDX: Argument 3 (e.g., count for read/write)
 * - ESI: Argument 4
 * - EDI: Argument 5
 *
 * Available System Calls:
 * - SYSCALL_EXIT   (1):  Exit user program (status in EBX)
 * - SYSCALL_READ   (3):  Read from fd (EBX=fd, ECX=buf, EDX=count)
 * - SYSCALL_WRITE  (4):  Write to fd (EBX=fd, ECX=buf, EDX=count)
 */
#include <stdio.h>
#include <stddef.h>

#include <kernel/syscall.h>

#include "include/interrupts.h"
#include "include/gdt.h"

/* Forward declaration of idt_set_gate (defined in idt.c) */
extern void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
/* Assembly stub for system call interrupt handler (defined in isr_stubs.nasm) */
extern void isr128(void);

/**
 * System call handler
 *
 * This is invoked when a user-mode program executes 'int 0x80'. The system call number is in EAX, and up to 5 arguments
 * can be passed in EBX, ECX, EDX, ESI, EDI.
 *
 * The handler:
 * 1. Validates the syscall number
 * 2. Performs the requested kernel operation
 * 3. Returns the result in EAX
 * 4. iret restores user mode execution
 *
 * @param r Pointer to saved CPU register state (on kernel stack)
 */
void syscall_handler(regs_t* r) {
    uint32_t syscall_num = r->eax;

    switch (syscall_num) {
        case SYSCALL_EXIT:
            {
                int exit_code = (int) r->ebx;
                printf("\n[SYSCALL] User program exited with code %d\n", exit_code);
                r->eax = 0;
                /* Loop forever in kernel mode instead of returning to user mode */
                while (1) {
                    asm volatile("hlt");
                }
            }
            break;

        case SYSCALL_READ:
            {
                extern int getchar(void);  /* Kernel getchar */
                int fd = (int) r->ebx;
                char *buf = (char*) r->ecx;
                size_t count = (size_t) r->edx;
                /* For now, only support stdin (fd = 0) */
                if (fd != 0) {
                    r->eax = (uint32_t) -1;  /* Error: unsupported fd */
                    break;
                }
                /* Read characters from keyboard */
                size_t i;
                for (i = 0; i < count; i++) {
                    int c = getchar();
                    if (c == -1) {
                        break;  /* EOF or error */
                    }
                    buf[i] = (char) c;
                }
                r->eax = i;  /* Return number of bytes read */
            }
            break;

        case SYSCALL_WRITE:
            {
                int fd = (int) r->ebx;
                const char *buf = (const char*) r->ecx;
                size_t count = (size_t) r->edx;
                /* For now, only support stdout (fd = 1) and stderr (fd = 2) */
                if (fd != 1 && fd != 2) {
                    r->eax = (uint32_t) - 1;  /* Error: unsupported fd */
                    break;
                }
                /* Write each character in the buffer */
                for (size_t i = 0; i < count; i++) {
                    printf("%c", buf[i]);
                }
                r->eax = count;  /* Return number of bytes written */
            }
            break;

        default:
            printf("[SYSCALL] Unknown system call: %u\n", syscall_num);
            r->eax = (uint32_t) - 1;  /* Error */
            break;
    }
}

/**
 * Initialize the system call interface.
 *
 * Registers interrupt 0x80 with DPL=3 (Ring 3) so user mode programs can call it.
 */
void syscall_init(void) {
    /* Register the system call handler for interrupt 0x80
     *
     * We use TRAP gate 0x80 (common convention from Linux). The key difference from normal interrupt gates is that we
     * set DPL=3 to allow user mode (Ring 3) programs to execute 'int 0x80'.
     *
     * IMPORTANT: We use a TRAP gate (not interrupt gate) so that interrupts remain enabled during syscalls.
     * This is crucial for blocking syscalls like READ that wait for keyboard input - the keyboard IRQ must be able
     * to fire while we're waiting!
     *
     * Difference between interrupt gate (0xEE) and trap gate (0xEF):
     * - Interrupt gate: CPU clears IF (disables interrupts) on entry
     * - Trap gate: CPU leaves IF unchanged (interrupts stay enabled)
     *
     * Gate flags: 0xEF
     * - P    = 1: Present (bit 7)
     * - DPL  = 11: Ring 3 access allowed (bits 6-5)
     * - S    = 0: System gate (bit 4, part of gate type)
     * - Type = 1111: 32-bit trap gate (bits 3-0)
     *
     * Result: 0b11101111 = 0xEF
     */
    const uint8_t flags_syscall_gate = 0xEF;  /* P=1, DPL=11 (Ring 3), type=1111 (TRAP gate) */
    const uint16_t kernel_code_selector = KERNEL_CS;
    /* Set up interrupt 0x80 with Ring 3 access */
    idt_set_gate(0x80, (uint32_t) isr128, kernel_code_selector, flags_syscall_gate);
    printf("[  OK  ] System call interface initialized (int 0x80, trap gate)\n");
}
