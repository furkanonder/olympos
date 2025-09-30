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

/* List of segments registered in GDT for x86 architecture. */
#define SEGMENT_UNUSED 0x0
#define SEGMENT_KCODE  0x1
#define SEGMENT_KDATA  0x2
#define SEGMENT_UCODE  0x3
#define SEGMENT_UDATA  0x4

#define NUM_SEGMENTS    5

#endif
