#ifndef ARCH_I386_INTERRUPTS_H
#define ARCH_I386_INTERRUPTS_H

#include <stdint.h>

/* Each interrupt/exception has a unique vector number (0-255) used as an IDT index.
 * Vectors 0-31: CPU-defined exceptions (Division by Zero, Page Fault, etc.)
 * Vectors 32-255: User-defined interrupts (hardware IRQs, software interrupts) */
#define IDT_NUM_ENTRIES 256

/**
 * IDT entry (also called IDT gate descriptor in Intel documentation).
 *
 *   63                                 48  47 46 45  44 43 42 41 40 39                          32
 *  +-------------------------------------+---+-----+---+-----------+-----------------------------+
 *  |               Offset 31..16         | P | DPL | 0 | Gate Type |          Reserved           |
 *  +-------------------------------------+---+-----+---+-----------+-----------------------------+
 *   31                                          16 15                                            0
 *  +----------------------------------------------+----------------------------------------------+
 *  |            Segment Selector                  |                Offset 15..0                  |
 *  +----------------------------------------------+----------------------------------------------+
 *
 * Fields in this struct map as follows:
 * - base_lo/base_hi:  Offset 15..0 and Offset 31..16 (handler address)
 * - selector:         Segment Selector (must select a valid code segment in GDT)
 * - zero:             Must be zero (reserved)
 * - type_attr:        [7] = P (Present), [6:5] = DPL, [4] = 0, [3:0] = Gate Type
 *
 * Gate Type: 0xE = 32-bit Interrupt Gate, 0xF = 32-bit Trap Gate
 * - Interrupt Gate (0xE): Clears IF flag, blocks nested interrupts. Used for hardware interrupts.
 * - Trap Gate (0xF): Preserves IF flag, allows nested interrupts. Used for software interrupts.
 *
 * DPL: A 2-bit value which defines the CPU Privilege Levels which are allowed to access this interrupt via the INT
 *      instruction.
 * P: Present bit. Must be set (1) for the descriptor to be valid.
 */
struct idt_entry {
	uint16_t base_lo;    /* Offset 15..0  */
	uint16_t selector;   /* Segment selector */
	uint8_t  zero;       /* Reserved, must be 0 */
	uint8_t  type_attr;  /* P | DPL | 0 | Gate Type */
	uint16_t base_hi;    /* Offset 31..16 */
} __attribute__((packed));
typedef struct idt_entry idt_entry_t;

/**
 * IDT Register (IDTR)
 *
 *   47                32 31                                             0
 *  +--------------------+-----------------------------------------------+
 *  |       Size - 1     |                    Base (linear)              |
 *  +--------------------+-----------------------------------------------+
 *
 * - limit: Size of the IDT in bytes minus one (because zero means 1 byte)
 * - base:  Linear address of the first IDT entry
 */
struct idt_register {
	uint16_t limit; /* size in bytes - 1 */
	uint32_t base;  /* linear address of first IDT entry */
} __attribute__((packed));
typedef struct idt_register idt_register_t;

/* ISR Stub Functions - each corresponds to a CPU exception vector (0-31: Division by Zero, Page Fault, etc.) */
void isr0();   /* Division By Zero */
void isr1();   /* Debug */
void isr2();   /* Non Maskable Interrupt */
void isr3();   /* Breakpoint */
void isr4();   /* Into Detected Overflow */
void isr5();   /* Out of Bounds */
void isr6();   /* Invalid Opcode */
void isr7();   /* No Coprocessor */
void isr8();   /* Double Fault */
void isr9();   /* Coprocessor Segment Overrun */
void isr10();  /* Bad TSS */
void isr11();  /* Segment Not Present */
void isr12();  /* Stack Fault */
void isr13();  /* General Protection Fault */
void isr14();  /* Page Fault */
void isr15();  /* Unknown Interrupt */
void isr16();  /* Coprocessor Fault */
void isr17();  /* Alignment Check */
void isr18();  /* Machine Check */
void isr19();  /* Reserved */
void isr20();  /* Reserved */
void isr21();  /* Reserved */
void isr22();  /* Reserved */
void isr23();  /* Reserved */
void isr24();  /* Reserved */
void isr25();  /* Reserved */
void isr26();  /* Reserved */
void isr27();  /* Reserved */
void isr28();  /* Reserved */
void isr29();  /* Reserved */
void isr30();  /* Reserved */
void isr31();  /* Reserved */

/* IRQ Stub Functions - hardware interrupts remapped to vectors 32-47 (0x20-0x2F) after PIC initialization */
void irq0();   /* Programmable Interval Timer (PIT) */
void irq1();   /* Keyboard */
void irq2();   /* Cascade (used internally by PICs) */
void irq3();   /* COM2 (Serial Port 2) */
void irq4();   /* COM1 (Serial Port 1) */
void irq5();   /* LPT2 (Parallel Port 2) */
void irq6();   /* Floppy Disk Controller */
void irq7();   /* LPT1 (Parallel Port 1) */
void irq8();   /* Real-Time Clock (RTC) */
void irq9();   /* ACPI / Available */
void irq10();  /* Available */
void irq11();  /* Available */
void irq12();  /* PS/2 Mouse */
void irq13();  /* FPU / Coprocessor / Inter-processor */
void irq14();  /* Primary ATA Hard Disk */
void irq15();  /* Secondary ATA Hard Disk */

/**
 * CPU register state structure
 * 
 * This structure represents the complete CPU state at the time of an interrupt.
 * It matches the exact order of registers as pushed by the assembly ISR stubs.
 *
 * The structure layout follows the exact sequence of operations in isr_stubs.nasm:
 * 
 * 1. CPU AUTOMATIC ACTIONS (Hardware Level):
 *    - Receives interrupt/exception
 *    - Automatically pushes: EFLAGS, CS, EIP, ErrorCode (if applicable)
 *    - Looks up handler in IDT using interrupt number as index
 *    - Jumps to the handler (our ISR stub)
 * 
 * 2. ISR STUB ACTIONS (Assembly Level):
 *    - Handles error codes correctly (some exceptions push them, others don't)
 *    - Pushes interrupt number for identification
 *    - Executes pushad instruction to save all general registers
 *    - Saves data segment (DS) - user space might have different segments
 *    - Switches to kernel segments for safe memory access
 *    - Calls C handler with pointer to saved state
 * 
 * 3. THE PUSHAD INSTRUCTION BEHAVIOR:
 *    The x86 pushad instruction pushes registers in this specific order:
 *    1. EAX, 2. ECX, 3. EDX, 4. EBX, 5. ESP (original), 6. EBP, 7. ESI, 8. EDI
 *
 * DETAILED STACK GROWTH VISUALIZATION (downward):
 * ===============================================
 * 
 * STEP 1 - Before Interrupt (Normal Execution):
 * ESP ──► [user data...]
 *         0x1000
 * 
 * STEP 2 - CPU Automatic Pushes (Hardware Level):
 * ESP ──► [EFLAGS] [CS] [EIP] [ErrorCode] [user data...]
 *         0x0FF0  ↑
 *                 CPU automatically pushed these 3-4 values
 *                 ErrorCode only pushed for specific exceptions (8,10,11,12,13,14,17)
 * 
 * STEP 3 - ISR Stub Manual Pushes (Assembly Level):
 * ESP ──► [IntNo] [EFLAGS] [CS] [EIP] [ErrorCode] [user data...]
 *         0x0FE8  ↑
 *                 ISR stub pushed interrupt number
 *                 This ESP value (0x0FE8) gets saved as ESP_DUMMY by pushad
 * 
 * STEP 4 - pushad Instruction (Saves All General Registers):
 * ESP ──► [EAX] [ECX] [EDX] [EBX] [ESP_DUMMY] [EBP] [ESI] [EDI] [IntNo] [EFLAGS] [CS] [EIP] [ErrorCode] [user data...]
 *         0x0FC8  ↑
 *                 Current ESP after pushad (useful for interrupt handling)
 *                 ↑
 *                 ESP_DUMMY contains 0x0FE8 (the ESP from BEFORE pushad - not useful!)
 * 
 * STEP 5 - Additional ISR Stub Pushes (DS, regs_ptr):
 * ESP ──► [DS] [regs_ptr] [EAX] [ECX] [EDX] [EBX] [ESP_DUMMY] [EBP] [ESI] [EDI] [IntNo] [EFLAGS] [CS] [EIP] [ErrorCode] [user data...]
 *         0x0FC0  ↑
 *                 This is the REAL current stack pointer (what you need for interrupt handling)
 *
 * ANALYSIS OF WHY ESP_DUMMY IS "DUMMY":
 * ==============================================
 * 
 * 1. ESP_DUMMY = 0x0FE8 (ESP from STEP 3 - before pushad executed)
 * 2. Current ESP = 0x0FC0 (ESP from STEP 5 - after all pushes completed)
 * 3. For interrupt handling, you need the CURRENT ESP (0x0FC0), not the old one (0x0FE8)
 * 4. ESP_DUMMY is just a placeholder to maintain the exact register order from pushad
 * 
 * =============================================================================
 * STRUCTURE LAYOUT
 * =============================================================================
 * 
 * Layout (matches pushad + manual pushes in isr_stubs.nasm):
 * - ds: Data segment selector (saved manually)
 * - edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax: General registers (pushad)
 * - int_no: Interrupt/exception number (0-31)
 * - err_code: Error code (if applicable, 0 otherwise)
 * - eip, cs, eflags: Return address and flags (pushed by CPU)
 * - useresp, ss: User stack pointer and segment (if privilege change)
 * 
 * MEMORY LAYOUT OF regs_t STRUCTURE:
 * ==================================
 * 
 * Offset  Field        Value                    Description
 * ------  -----------  -----------------------  -----------------------------------
 * +0      ds           0x10                    Data segment selector
 * +4      edi          [saved EDI]             General register
 * +8      esi          [saved ESI]             General register  
 * +12     ebp          [saved EBP]             General register
 * +16     esp_dummy    0x0FE8                  **DUMMY** - ESP before pushad
 * +20     ebx          [saved EBX]             General register
 * +24     edx          [saved EDX]             General register
 * +28     ecx          [saved ECX]             General register
 * +32     eax          [saved EAX]             General register
 * +36     int_no       [interrupt number]      Interrupt/exception number
 * +40     err_code     [error code]            Error code (if applicable)
 * +44     eip          [return address]        Instruction pointer
 * +48     cs           [code segment]          Code segment selector
 * +52     eflags       [CPU flags]             CPU flags register
 * +56     useresp      [user stack pointer]    User stack (if privilege change)
 * +60     ss           [stack segment]         Stack segment (if privilege change)
 * 
 * VISUAL STACK LAYOUT:
 * ====================
 * 
 * High Memory (0x1000)
 *     │
 *     ▼ [user data...]
 *     │
 *     ▼ [EFLAGS]      ← CPU pushed
 *     │ [CS]          ← CPU pushed  
 *     │ [EIP]         ← CPU pushed
 *     │ [ErrorCode]   ← CPU pushed (some exceptions)
 *     │ [IntNo]       ← ISR stub pushed
 *     │
 *     ▼ [EAX]         ← pushad
 *     │ [ECX]         ← pushad
 *     │ [EDX]         ← pushad
 *     │ [EBX]         ← pushad
 *     │ [ESP_DUMMY]   ← pushad (contains 0x0FE8 - NOT current ESP!)
 *     │ [EBP]         ← pushad
 *     │ [ESI]         ← pushad
 *     │ [EDI]         ← pushad
 *     │
 *     ▼ [DS]          ← ISR stub pushed
 *     │ [regs_ptr]    ← ISR stub pushed
 *     │
 *     ▼ [current data...]
 *     │
 * Low Memory (0x0FC0) ← CURRENT ESP (what you actually need)
 */
typedef struct regs {
    uint32_t ds;                                              /* Data segment selector */
    uint32_t edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax;    /* General registers */
    uint32_t int_no, err_code;                                /* Interrupt number and error code */
    uint32_t eip, cs, eflags, useresp, ss;                    /* Return address and stack info */
} regs_t;

/**
 * ISR handler function pointer type
 * 
 * @param r Pointer to saved CPU register state
 */
typedef void (*isr_handler_fn)(regs_t*);

/**
 * Register a custom handler for an ISR
 *
 * Allows modules to register custom exception handlers instead of using
 * the default panic behavior. This enables proper exception handling
 * for things like page faults, general protection faults, etc.
 *
 * @param isr ISR number (0-31)
 * @param handler Function to call when this ISR occurs
 * @return 0 on success, -1 on invalid ISR number
 */
int register_isr(int isr, isr_handler_fn handler);

/**
 * Unregister a handler and mask the IRQ at the PIC.
 *
 * This function removes the registered handler for the specified IRQ and masks (disables)
 * the interrupt at the PIC level, preventing further interrupts from this line.
 *
 * @param irq Hardware IRQ line number (0..15 on legacy PIC)
 * @return 0 on success, -1 if the IRQ is out of range
 */
int unregister_irq(int irq);

/**
 * Main ISR handler dispatcher
 *
 * This function is called by the assembly ISR stubs with a pointer to
 * the saved CPU state. It either dispatches to a registered custom
 * handler or shows a panic message for unhandled exceptions.
 *
 * @param r Pointer to saved CPU register state
 */
void isr_handler(regs_t* r);

/**
 * Initialize and load the IDT.
 *
 * Sets up the Interrupt Descriptor Table with handlers for CPU exceptions (vectors 0-31) and hardware
 * interrupts (vectors 32-47). Each IDT entry contains a handler address, code segment selector, and
 * type/attributes. When an interrupt occurs, the CPU uses the vector number to index into the IDT and
 * jump to the corresponding handler.
 *
 * Steps performed:
 * - Clears all 256 IDT entries
 * - Installs ISR handlers for CPU exceptions (0-31)
 * - Remaps PIC to vectors 32-47 to avoid conflicts with CPU exceptions
 * - Installs IRQ handlers for hardware interrupts (32-47)
 * - Loads the IDTR register and enables interrupts
 *
 * Vector assignments:
 * - 0-31:   CPU exceptions (division error, page fault, general protection fault, etc.)
 * - 32-47:  Hardware interrupts (PIT, keyboard, serial ports, disk controllers, etc.)
 */
void idt_init(void);

#endif
