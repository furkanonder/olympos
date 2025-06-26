#ifndef _KERNEL_GDT_H
#define _KERNEL_GDT_H

#include <stdint.h>

/* Segmentation provides a mechanism for dividing the processor’s addressable memory space (called the linear address
 * space) into smaller protected address spaces called segments. Segments can be used to hold the code, data, and stack
 * for a program or to hold system data structures (such as a TSS or LDT).
 *
 * If more than one program (or task) is running on a processor, each program can be assigned its own set of segments.
 * The processor then enforces the boundaries between these segments and insures that one program does not interfere
 * with the execution of another program by writing into the other program’s segments. The segmentation mechanism also
 * allows typing of segments so that the operations that may be performed on a particular type of segment can be
 * restricted.
 *
 * To enable segmentation we need to set up a table that describes each segment - a segment descriptor table. In x86,
 * there are two types of descriptor tables: the Global Descriptor Table (GDT) and Local Descriptor Tables (LDT).
 * An LDT is set up and managed by user-space processes, and all processes have their own LDT. LDTs can be used if
 * a more complex segmentation model is desired - we won’t use it. The GDT is shared by everyone - it’s global. */

/**
 * GDT entry format.
 * Check out https://wiki.osdev.org/Global_Descriptor_Table for detailed anatomy of fields.
 */
struct gdt_entry {
    uint16_t limit_lo;          /* Limit 0:15. */
    uint16_t base_lo;           /* Base 0:15. */
    uint8_t  base_mi;           /* Base 16:23. */
    uint8_t  access;            /* Access Byte. */
    uint8_t  limit_hi_flags;    /* Limit 16:19 | FLags. */
    uint8_t  base_hi;           /* Base 24:31. */
} __attribute__((packed));
typedef struct gdt_entry gdt_entry_t;

/**
 * 48-bit GDTR address register format.
 * Used for loading the GDT table with `lgdt` instruction.
 */
struct gdt_register {
    uint16_t boundary;  /* Boundary */
    uint32_t base;      /* GDT base address */
} __attribute__((packed));
typedef struct gdt_register gdt_register_t;

/* List of segments registered in GDT. */
#define SEGMENT_UNUSED 0x0
#define SEGMENT_KCODE  0x1
#define SEGMENT_KDATA  0x2
#define SEGMENT_UCODE  0x3
#define SEGMENT_UDATA  0x4

#define NUM_SEGMENTS 	5

void gdt_init();

#endif
