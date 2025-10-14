#ifndef ARCH_I386_GDT_H
#define ARCH_I386_GDT_H

#include <stdint.h>

/* Segmentation provides a mechanism for dividing the processor's addressable memory space (called the linear address
 * space) into smaller protected address spaces called segments. Segments can be used to hold the code, data, and stack
 * for a program or to hold system data structures (such as a TSS or LDT).
 *
 * If more than one program (or task) is running on a processor, each program can be assigned its own set of segments.
 * The processor then enforces the boundaries between these segments and ensures that one program does not interfere
 * with the execution of another program by writing into the other program's segments. The segmentation mechanism also
 * allows typing of segments so that the operations that may be performed on a particular type of segment can be
 * restricted.
 *
 * To enable segmentation, we need to set up a table that describes each segment - a segment descriptor table. In x86,
 * there are two types of descriptor tables: the Global Descriptor Table (GDT) and Local Descriptor Tables (LDT).
 * An LDT is set up and managed by user-space processes, and all processes have their own LDT. LDTs can be used if
 * a more complex segmentation model is desired - we won't use it. The GDT is shared by everyone - it's global. */

 /**
 * GDT entry (Also called segment descriptor in Intel documentation).
 *
 *   31                 24  23 22  21  20 19                  16  15  14 13  12  11  10    9   8  7                    0
 *  +---------------------+---+--+---+---+----------------------+----+-----+---+---+----+----+---+---------------------+
 *  | Base Address 31..24 | G |DB| L |AVL| Segment Limit 19..16 | P  | DPL | S | E | DC | RW | A | Base Address 23..16 |
 *  +---------------------+---+--+---+---+----------------------+----+-----+---+---+----+----+---+---------------------+
 *   63                 56 55  54 53  52  51                  48  47  46 45 44  43   42   41  40  39                  32
 *
 *   31                                                       16 15                                                    0
 *  +-----------------------------------------------------------+------------------------------------------------------+
 *  |                    Base Address 15..0                     |                   Segment Limit 15..0                |
 *  +-----------------------------------------------------------+------------------------------------------------------+
 *   47                                                       32 31                                                   16
 *
 * Fields in this struct map as follows:
 * - limit_lo:        Limit 15..0 (lower 16 bits of segment size limit)
 * - base_lo:         Base 15..0 (lower 16 bits of segment base address)
 * - base_mi:         Base 23..16 (middle 8 bits of segment base address)
 * - access:          Access Byte [P|DPL|S|E|DC|RW|A] (bits 47..40)
 * - limit_hi_flags:  Limit 19..16 (upper 4 bits) | Flags [G|DB|L|AVL] (bits 55..48)
 * - base_hi:         Base 31..24 (upper 8 bits of segment base address)
 *
 * Access Byte (bits 47..40):
 * - P   (Present, bit 47):                           Must be 1 for valid segment
 * - DPL (Descriptor Privilege Level, bits 46..45):   Ring level (0 = kernel, 3 = user)
 * - S   (Descriptor Type, bit 44):                   1 = code/data segment, 0 = system segment
 * - E   (Executable, bit 43):                        1 = code segment, 0 = data segment
 * - DC  (Direction/Conforming, bit 42):              For data: 0=grows up, 1=grows down; For code: conforming bit
 * - RW  (Readable/Writable, bit 41):                 For code: readable; For data: writable
 * - A   (Accessed, bit 40):                          Set by CPU when segment is accessed
 *
 * Flags (bits 55..52):
 * - G   (Granularity, bit 55):                      0 = 1 byte granularity, 1 = 4 KiB granularity
 * - DB  (Default/Big, bit 54):                      0 = 16-bit segment, 1 = 32-bit segment
 * - L   (Long-mode, bit 53):                        1 = 64-bit code segment (must be 0 for 32-bit)
 * - AVL (Available, bit 52):                        Available for system software use
 */
struct gdt_entry {
    uint16_t limit_lo;          /* Bits 15..0:   Limit 15..0 (segment size limit) */
    uint16_t base_lo;           /* Bits 31..16:  Base 15..0 (segment base address) */
    uint8_t  base_mi;           /* Bits 39..32:  Base 23..16 (segment base address) */
    uint8_t  access;            /* Bits 47..40:  Access Byte [P|DPL|S|E|DC|RW|A] */
    uint8_t  limit_hi_flags;    /* Bits 55..48:  Limit 19..16 | Flags [G|D/B|L|AVL] */
    uint8_t  base_hi;           /* Bits 63..56:  Base 31..24 (segment base address) */
} __attribute__((packed));
typedef struct gdt_entry gdt_entry_t;

/**
 * 48-bit GDTR (Global Descriptor Table Register) format for x86 architecture.
 *
 *   47                                32 31                                            0
 *  +------------------------------------+----------------------------------------------+
 *  |              Reserved              |              GDT Base Address                |
 *  +------------------------------------+----------------------------------------------+
 *   15                                 0
 *  +-----------------------------------+
 *  |              GDT Limit (Size)     |
 *  +-----------------------------------+
 *
 * Fields in this struct map as follows:
 * - boundary:  GDT Limit (15:0)         - Size of GDT table in bytes minus 1
 * - base:      GDT Base Address (31:0)  - Linear address of the GDT table
 */
struct gdt_register {
    uint16_t boundary;  /* GDT Limit (15:0)         - Size of GDT table in bytes minus 1 */
    uint32_t base;      /* GDT Base Address (31:0)  - Linear address of the GDT table */
} __attribute__((packed));
typedef struct gdt_register gdt_register_t;

/**
 * TSS (Task State Segment) structure for x86 32-bit architecture.
 *
 * The TSS is a special hardware-defined structure used by the x86 CPU for two purposes:
 * 1. Hardware task switching (rarely used in modern OSes - we don't use it)
 * 2. Privilege level transitions (Ring 3 → Ring 0) - THIS IS WHAT WE USE IT FOR
 *
 * When a privilege level change occurs (interrupt, syscall, exception from user mode), the CPU:
 * 1. Reads SS0 and ESP0 from the TSS
 * 2. Switches to that kernel stack
 * 3. Pushes the user-mode context (SS, ESP, EFLAGS, CS, EIP) onto the kernel stack
 * 4. Jumps to the interrupt/syscall handler
 *
 * Memory Layout (104 bytes total):
 *  Offset  Size  Field       Description
 *  +------+-----+----------+------------------------------------------------------------------+
 *  | 0x00 |  4  | prev_tss | Previous TSS selector (for hardware task switching, unused)      |
 *  | 0x04 |  4  | esp0     | Stack pointer for Ring 0 (KERNEL STACK - CRITICAL!)              |
 *  | 0x08 |  4  | ss0      | Stack segment for Ring 0 (KERNEL_DS)                             |
 *  | 0x0C |  4  | esp1     | Stack pointer for Ring 1 (unused - most OSes don't use Ring 1/2) |
 *  | 0x10 |  4  | ss1      | Stack segment for Ring 1 (unused)                                |
 *  | 0x14 |  4  | esp2     | Stack pointer for Ring 2 (unused)                                |
 *  | 0x18 |  4  | ss2      | Stack segment for Ring 2 (unused)                                |
 *  | 0x1C |  4  | cr3      | Page Directory Base Register (PDBR) for hardware task switching |
 *  | 0x20 |  4  | eip      | Instruction pointer (saved on task switch)                       |
 *  | 0x24 |  4  | eflags   | CPU flags register (saved on task switch)                        |
 *  | 0x28 |  4  | eax      | General purpose register (saved on task switch)                  |
 *  | 0x2C |  4  | ecx      | General purpose register (saved on task switch)                  |
 *  | 0x30 |  4  | edx      | General purpose register (saved on task switch)                  |
 *  | 0x34 |  4  | ebx      | General purpose register (saved on task switch)                  |
 *  | 0x38 |  4  | esp      | Stack pointer (saved on task switch)                             |
 *  | 0x3C |  4  | ebp      | Base pointer (saved on task switch)                              |
 *  | 0x40 |  4  | esi      | Source index (saved on task switch)                              |
 *  | 0x44 |  4  | edi      | Destination index (saved on task switch)                         |
 *  | 0x48 |  4  | es       | Extra segment selector (saved on task switch)                    |
 *  | 0x4C |  4  | cs       | Code segment selector (saved on task switch)                     |
 *  | 0x50 |  4  | ss       | Stack segment selector (saved on task switch)                    |
 *  | 0x54 |  4  | ds       | Data segment selector (saved on task switch)                     |
 *  | 0x58 |  4  | fs       | FS segment selector (saved on task switch)                       |
 *  | 0x5C |  4  | gs       | GS segment selector (saved on task switch)                       |
 *  | 0x60 |  4  | ldt      | Local Descriptor Table selector (for LDT, unused)                |
 *  | 0x64 |  2  | trap     | Debug trap flag: if set, causes debug exception on task switch   |
 *  | 0x66 |  2  | iomap    | I/O Map Base Address (offset to I/O permission bitmap)           |
 *  +------+-----+----------+------------------------------------------------------------------+
 *
 * IMPORTANT NOTES:
 * - In modern OSes, only esp0/ss0 fields are typically used (for Ring 3 → Ring 0 transitions)
 * - Hardware task switching is NOT used (we do software context switching instead)
 * - The CPU automatically loads esp0/ss0 when transitioning from Ring 3 to Ring 0
 * - For multitasking OSes, esp0 must be updated on every context switch to point to the
 *   current task's kernel stack
 *
 * @see Intel® 64 and IA-32 Architectures Software Developer's Manual, Volume 3A, Chapter 7
 * @see https://wiki.osdev.org/Task_State_Segment
 */
struct tss_entry {
    uint32_t prev_tss;      /* 0x00: Previous TSS selector (hardware task switching chain)             */
    uint32_t esp0;          /* 0x04: Stack pointer for Ring 0 (loaded on Ring 3 → Ring 0 transition)  */
    uint32_t ss0;           /* 0x08: Stack segment for Ring 0 (loaded on Ring 3 → Ring 0 transition)  */
    uint32_t esp1;          /* 0x0C: Stack pointer for Ring 1 (unused by most OSes)                   */
    uint32_t ss1;           /* 0x10: Stack segment for Ring 1 (unused by most OSes)                   */
    uint32_t esp2;          /* 0x14: Stack pointer for Ring 2 (unused by most OSes)                   */
    uint32_t ss2;           /* 0x18: Stack segment for Ring 2 (unused by most OSes)                   */
    uint32_t cr3;           /* 0x1C: Page Directory Base Register (for hardware task switching)       */
    uint32_t eip;           /* 0x20: Instruction pointer (saved state for hardware task switching)    */
    uint32_t eflags;        /* 0x24: CPU flags register (saved state for hardware task switching)     */
    uint32_t eax;           /* 0x28: General purpose register (saved state)                           */
    uint32_t ecx;           /* 0x2C: General purpose register (saved state)                           */
    uint32_t edx;           /* 0x30: General purpose register (saved state)                           */
    uint32_t ebx;           /* 0x34: General purpose register (saved state)                           */
    uint32_t esp;           /* 0x38: Stack pointer (saved state for hardware task switching)          */
    uint32_t ebp;           /* 0x3C: Base pointer (saved state for hardware task switching)           */
    uint32_t esi;           /* 0x40: Source index register (saved state for hardware task switching)  */
    uint32_t edi;           /* 0x44: Destination index register (saved state)                         */
    uint32_t es;            /* 0x48: Extra segment selector (saved state for hardware task switching) */
    uint32_t cs;            /* 0x4C: Code segment selector (saved state for hardware task switching)  */
    uint32_t ss;            /* 0x50: Stack segment selector (saved state)                             */
    uint32_t ds;            /* 0x54: Data segment selector (saved state for hardware task switching)  */
    uint32_t fs;            /* 0x58: FS segment selector (saved state for hardware task switching)    */
    uint32_t gs;            /* 0x5C: GS segment selector (saved state for hardware task switching)    */
    uint32_t ldt;           /* 0x60: Local Descriptor Table selector (for per-task LDT, unused)       */
    uint16_t trap;          /* 0x64: Debug trap flag (bit 0: trap on task switch for debugging)       */
    uint16_t iomap_base;    /* 0x66: I/O Map Base Address (offset to I/O permission bitmap)           */
} __attribute__((packed));
typedef struct tss_entry tss_entry_t;

/* List of segments registered in GDT for x86 architecture. */
#define SEGMENT_UNUSED 0x0
#define SEGMENT_KCODE  0x1
#define SEGMENT_KDATA  0x2
#define SEGMENT_UCODE  0x3
#define SEGMENT_UDATA  0x4
#define SEGMENT_TSS    0x5

#define NUM_SEGMENTS    6

/* Segment selectors with proper RPL (Requested Privilege Level) encoding.
 *
 * Segment Selector Format (16 bits):
 *   15                           3  2  1    0
 *  +-----------------------------+----+-----+
 *  |       Index (13 bits)       | TI | RPL |
 *  +-----------------------------+----+-----+
 *
 * Fields:
 * - Index (bits 15-3):  Index into GDT/LDT (which descriptor to use)
 * - TI    (bit 2):      Table Indicator (0 = GDT, 1 = LDT)
 * - RPL   (bits 1-0):   Requested Privilege Level (0 = Ring 0 [kernel], 3 = Ring 3 [user])
 *
 * Formula to compute selector: (index << 3) | (TI << 2) | RPL
 *
 * Examples:
 * - SEGMENT_KCODE (1) → (1 << 3) | 0 | 0 = 0x08 (kernel code selector)
 * - SEGMENT_KDATA (2) → (2 << 3) | 0 | 0 = 0x10 (kernel data selector)
 * - SEGMENT_UCODE (3) → (3 << 3) | 0 | 3 = 0x1B (user code selector, Ring 3)
 * - SEGMENT_UDATA (4) → (4 << 3) | 0 | 3 = 0x23 (user data selector, Ring 3)
 */
#define KERNEL_CS ((SEGMENT_KCODE << 3) | 0x0)  /* 0x08: Kernel code segment, Ring 0, GDT */
#define KERNEL_DS ((SEGMENT_KDATA << 3) | 0x0)  /* 0x10: Kernel data segment, Ring 0, GDT */

#endif
