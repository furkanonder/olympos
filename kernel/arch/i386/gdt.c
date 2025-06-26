#include <stdio.h>

#include <kernel/gdt.h>

// Defined in gdt_load.nasm We use this to properly reload the new segment registers.
extern void gdt_load(uint32_t);

static gdt_entry_t gdt[NUM_SEGMENTS];
static gdt_register_t gdtr;

/**
 * Setup one GDT entry.
 */
static void gdt_set_entry(int idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    gdt[idx].limit_lo       =  (uint16_t) (limit & 0xFFFF);
    gdt[idx].base_lo        =  (uint16_t) (base & 0xFFFF);
    gdt[idx].base_mi        =  (uint8_t) ((base >> 16) & 0xFF);
    gdt[idx].access         =  (uint8_t) access;
    gdt[idx].limit_hi_flags =  (uint8_t) ((limit >> 16) & 0x0F);
    gdt[idx].limit_hi_flags |= (uint8_t) (flags & 0xF0);
    gdt[idx].base_hi        =  (uint8_t) ((base >> 24) & 0xFF);
}

/**
 * Initialize the global descriptor table (GDT) by setting up the 5 entries of GDT, setting the GDTR register
 * to point to our GDT address, and then (through assembly `lgdt` instruction) load our GDT.
 *
 * - Entry 0: Null descriptor     (required by x86 architecture)
 * - Entry 1: Kernel code segment (0x00000000 - 0xFFFFFFFF, executable, ring 0)
 * - Entry 2: Kernel data segment (0x00000000 - 0xFFFFFFFF, read/write, ring 0)
 * - Entry 3: User code segment   (0x00000000 - 0xFFFFFFFF, executable, ring 3)
 * - Entry 4: User data segment   (0x00000000 - 0xFFFFFFFF, read/write, ring 3)
 */
void gdt_init() {
    /**
     * First, see https://wiki.osdev.org/Global_Descriptor_Table for a detailed anatomy of Access Byte and Flags fields
     * 
     * Access Byte -
     *   - P     = 1: present, must be 1 for valid selectors
     *   - DPL   = ?: ring level, 0 for kernel and 3 for user mode
     *   - S     = 1: should be 1 for all non-system segments
     *   - E     = ?: executable, 1 for code and 0 for data segment
     *   - DC =
     *   	- Direction bit for data selectors  = 0: segment spans up
     *   	- Conforming bit for code selectors = 0: can only be executed from ring level set in `DPL` field
     *   - RW =
     *   	- Readable bit for code selectors, = 1: allow reading
     *      - Writable bit for data selectors, = 1: allow writing
     *   - A     = 0: access bit, CPU sets it to 1 when accessing it
     *
     * Flags -
     *   - G  = 1  | Granularity flag. When flag is set(1), the segment limit is interpreted in 4-KByte units.
     * 	 - DB = 1  | Default operation size/Default stack pointer size flag. This flag should always be set to 1
	 *             | for 32-bit code segments.
     * 	 - L  = 0  | 64-bit code segment flag. A value of 0 indicates the instructions in this code segment are
     * 				 executed in compatibility mode.
     *    Hence, 0b110 -> 0xC0
     */

    // The first descriptor in the GDT is always a null descriptor and can never be used to access memory.
    gdt_set_entry(SEGMENT_UNUSED, 0u, 0u, 0u, 0u);
    /* Access:
     * Bit:     |  7  |  6  5 |  4  |  3  |  2  |  1  |  0  |
     * Content: |  P  |  DPL  |  S  |  E  |  DC |  RW |  A  |
     * Value:   |  1  |  0 0  |  1  |  1  |  0  |  1  |  0  | = 1001 1010 = 0x9A
    */
    gdt_set_entry(SEGMENT_KCODE, 0u, 0xFFFFF, 0x9A, 0xC0);
    /* Access:
     * Bit:     |  7  |  6  5 |  4  |  3  |  2  |  1  |  0  |
     * Content: |  P  |  DPL  |  S  |  E  |  DC |  RW |  A  |
     * Value:   |  1  |  0 0  |  1  |  0  |  0  |  1  |  0  | = 1001 0010 = 0x92
    */
    gdt_set_entry(SEGMENT_KDATA, 0u, 0xFFFFF, 0x92, 0xC0);
    /* Access:
     * Bit:     |  7  |  6  5 |  4  |  3  |  2  |  1  |  0  |
     * Content: |  P  |  DPL  |  S  |  E  |  DC |  RW |  A  |
     * Value:   |  1  |  1 1  |  1  |  1  |  0  |  1  |  0  | = 1111 1010 = 0xFA
    */
    gdt_set_entry(SEGMENT_UCODE, 0u, 0xFFFFF, 0xFA, 0xC0);
    /* Access:
     * Bit:     |  7  |  6  5 |  4  |  3  |  2  |  1  |  0  |
     * Content: |  P  |  DPL  |  S  |  E  |  DC |  RW |  A  |
     * Value:   |  1  |  1 1  |  1  |  0  |  0  |  1  |  0  | = 1111 0010 = 0xF2
    */
    gdt_set_entry(SEGMENT_UDATA, 0u, 0xFFFFF, 0xF2, 0xC0);

    /* Setup the GDTR register value. */
    gdtr.boundary = (sizeof(gdt_entry_t) * NUM_SEGMENTS) - 1;
    gdtr.base     = (uint32_t) &gdt;

    // Load the new GDT
    gdt_load((uint32_t) &gdtr);

    printf("[  OK  ] GDT initialized successfully.\n");
}
