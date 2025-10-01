#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "include/interrupts.h"
#include "include/io.h"
#include "include/pic.h"

/* Defined in idt_load.nasm. Used to load the IDTR (IDT Register). */
extern void idt_load(uint32_t);

/* Interrupt Descriptor Table: Array of 256 IDT entries (0 - 255) for exception and interrupt handlers */
static idt_entry_t idt[IDT_NUM_ENTRIES];
/* IDT Register: Contains the limit (size - 1) and base address of the IDT for the lidt instruction */
static idt_register_t idtr;

/**
 * Set a single IDT gate.
 *
 * @param num   Vector number (0..255)
 * @param base  32-bit handler address
 * @param sel   Segment selector (e.g., kernel code selector)
 * @param flags Type/flags byte: [7]=P (Present), [6:5]=DPL, [4]=0, [3:0]=Gate Type
 */
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
	idt[num].base_lo  = (uint16_t)(base & 0xFFFF);
	idt[num].selector = sel;
	idt[num].zero     = 0;
	idt[num].type_attr = flags;
	idt[num].base_hi  = (uint16_t)((base >> 16) & 0xFFFF);
}

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
void idt_init(void) {
	/* Initialize all 256 IDT entries to zero to ensure clean state. This prevents any garbage values from causing
	 * unexpected behavior. */
	memset(idt, 0, sizeof(idt));

	const uint8_t flags_int_gate = 0x8E;        		/* P=1, DPL=00, type=1110 (32-bit interrupt gate) */
	const uint16_t kernel_code_selector = (1u << 3);	/* GDT index 1 << 3 = 0x08 */

	/* These are the CPU-defined exceptions that can occur during execution. Each entry points to an assembly stub that
	 * handles the complete context saving and restoration sequence. */
	idt_set_gate(0,  (uint32_t)isr0,  kernel_code_selector, flags_int_gate);   /* Division By Zero */
	idt_set_gate(1,  (uint32_t)isr1,  kernel_code_selector, flags_int_gate);   /* Debug */
	idt_set_gate(2,  (uint32_t)isr2,  kernel_code_selector, flags_int_gate);   /* Non Maskable Interrupt */
	idt_set_gate(3,  (uint32_t)isr3,  kernel_code_selector, flags_int_gate);   /* Breakpoint */
	idt_set_gate(4,  (uint32_t)isr4,  kernel_code_selector, flags_int_gate);   /* Into Detected Overflow */
	idt_set_gate(5,  (uint32_t)isr5,  kernel_code_selector, flags_int_gate);   /* Out of Bounds */
	idt_set_gate(6,  (uint32_t)isr6,  kernel_code_selector, flags_int_gate);   /* Invalid Opcode */
	idt_set_gate(7,  (uint32_t)isr7,  kernel_code_selector, flags_int_gate);   /* No Coprocessor */
	idt_set_gate(8,  (uint32_t)isr8,  kernel_code_selector, flags_int_gate);   /* Double Fault */
	idt_set_gate(9,  (uint32_t)isr9,  kernel_code_selector, flags_int_gate);   /* Coprocessor Segment Overrun */
	idt_set_gate(10, (uint32_t)isr10, kernel_code_selector, flags_int_gate);   /* Bad TSS */
	idt_set_gate(11, (uint32_t)isr11, kernel_code_selector, flags_int_gate);   /* Segment Not Present */
	idt_set_gate(12, (uint32_t)isr12, kernel_code_selector, flags_int_gate);   /* Stack Fault */
	idt_set_gate(13, (uint32_t)isr13, kernel_code_selector, flags_int_gate);   /* General Protection Fault */
	idt_set_gate(14, (uint32_t)isr14, kernel_code_selector, flags_int_gate);   /* Page Fault */
	idt_set_gate(15, (uint32_t)isr15, kernel_code_selector, flags_int_gate);   /* Unknown Interrupt */
	idt_set_gate(16, (uint32_t)isr16, kernel_code_selector, flags_int_gate);   /* Coprocessor Fault */
	idt_set_gate(17, (uint32_t)isr17, kernel_code_selector, flags_int_gate);   /* Alignment Check */
	idt_set_gate(18, (uint32_t)isr18, kernel_code_selector, flags_int_gate);   /* Machine Check */
	idt_set_gate(19, (uint32_t)isr19, kernel_code_selector, flags_int_gate);   /* Reserved */
	idt_set_gate(20, (uint32_t)isr20, kernel_code_selector, flags_int_gate);   /* Reserved */
	idt_set_gate(21, (uint32_t)isr21, kernel_code_selector, flags_int_gate);   /* Reserved */
	idt_set_gate(22, (uint32_t)isr22, kernel_code_selector, flags_int_gate);   /* Reserved */
	idt_set_gate(23, (uint32_t)isr23, kernel_code_selector, flags_int_gate);   /* Reserved */
	idt_set_gate(24, (uint32_t)isr24, kernel_code_selector, flags_int_gate);   /* Reserved */
	idt_set_gate(25, (uint32_t)isr25, kernel_code_selector, flags_int_gate);   /* Reserved */
	idt_set_gate(26, (uint32_t)isr26, kernel_code_selector, flags_int_gate);   /* Reserved */
	idt_set_gate(27, (uint32_t)isr27, kernel_code_selector, flags_int_gate);   /* Reserved */
	idt_set_gate(28, (uint32_t)isr28, kernel_code_selector, flags_int_gate);   /* Reserved */
	idt_set_gate(29, (uint32_t)isr29, kernel_code_selector, flags_int_gate);   /* Reserved */
	idt_set_gate(30, (uint32_t)isr30, kernel_code_selector, flags_int_gate);   /* Reserved */
	idt_set_gate(31, (uint32_t)isr31, kernel_code_selector, flags_int_gate);   /* Reserved */
	/* Remap PIC. The PIC is remapped to avoid conflicts with CPU exceptions (0-31).
	 * IRQs 0-7 (master PIC) are remapped to vectors 32-39 (0x20-0x27).
	 * IRQs 8-15 (slave PIC) are remapped to vectors 40-47 (0x28-0x2F). */
	pic_remap(0x20, 0x28);
	/* Hardware interrupt requests from the Programmable Interrupt Controller (PIC). These are triggered by external
	 * hardware devices and are mapped to standard PC interrupt lines. */
	idt_set_gate(32, (uint32_t)irq0,  kernel_code_selector, flags_int_gate);   /* Programmable Interval Timer (PIT) */
	idt_set_gate(33, (uint32_t)irq1,  kernel_code_selector, flags_int_gate);   /* Keyboard */
	idt_set_gate(34, (uint32_t)irq2,  kernel_code_selector, flags_int_gate);   /* Cascade (used internally by PICs) */
	idt_set_gate(35, (uint32_t)irq3,  kernel_code_selector, flags_int_gate);   /* COM2 (Serial Port 2) */
	idt_set_gate(36, (uint32_t)irq4,  kernel_code_selector, flags_int_gate);   /* COM1 (Serial Port 1) */
	idt_set_gate(37, (uint32_t)irq5,  kernel_code_selector, flags_int_gate);   /* LPT2 (Parallel Port 2) */
	idt_set_gate(38, (uint32_t)irq6,  kernel_code_selector, flags_int_gate);   /* Floppy Disk Controller */
	idt_set_gate(39, (uint32_t)irq7,  kernel_code_selector, flags_int_gate);   /* LPT1 (Parallel Port 1) */
	idt_set_gate(40, (uint32_t)irq8,  kernel_code_selector, flags_int_gate);   /* Real-Time Clock (RTC) */
	idt_set_gate(41, (uint32_t)irq9,  kernel_code_selector, flags_int_gate);   /* ACPI / Available */
	idt_set_gate(42, (uint32_t)irq10, kernel_code_selector, flags_int_gate);   /* Available */
	idt_set_gate(43, (uint32_t)irq11, kernel_code_selector, flags_int_gate);   /* Available */
	idt_set_gate(44, (uint32_t)irq12, kernel_code_selector, flags_int_gate);   /* PS/2 Mouse */
	idt_set_gate(45, (uint32_t)irq13, kernel_code_selector, flags_int_gate);   /* FPU / Coprocessor / Inter-processor */
	idt_set_gate(46, (uint32_t)irq14, kernel_code_selector, flags_int_gate);   /* Primary ATA Hard Disk */
	idt_set_gate(47, (uint32_t)irq15, kernel_code_selector, flags_int_gate);   /* Secondary ATA Hard Disk */

	/* The IDTR tells the CPU where to find the IDT and how big it is.
	 * - limit: Offset of the last valid byte in the IDT (size - 1)
	 * - base: Linear address of the first IDT entry
	 * After this, the CPU knows where to look when interrupts occur. */
	idtr.limit = (uint16_t)(sizeof(idt_entry_t) * IDT_NUM_ENTRIES - 1);
	idtr.base  = (uint32_t)&idt[0];
	idt_load((uint32_t)&idtr);

	/* This allows the CPU to respond to interrupts. Without this, the CPU will ignore all interrupts except
	 * NMI (Non-Maskable Interrupt). */
	printf("[  OK  ] IDT initialized successfully.\n");
	asm volatile ("sti");  /* Enable interrupts globally */
}
