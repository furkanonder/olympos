/**
 * Intel 8259A Programmable Interrupt Controller (PIC) driver.
 *
 * The 8259A PIC is a chip that manages hardware interrupts (IRQs) from devices like the keyboard,
 * timer, and disk controllers. In a standard PC, two PICs are cascaded together:
 * - Master PIC (PIC1): Handles IRQs 0-7
 * - Slave PIC (PIC2): Handles IRQs 8-15, connected to Master's IRQ 2
 *
 * The PICs must be "remapped" during kernel initialization because their default IRQ mappings
 * (IRQ 0-7 -> INT 0x08-0x0F, IRQ 8-15 -> INT 0x70-0x77) conflict with CPU exception vectors.
 * We remap them to INT 0x20-0x2F to avoid these conflicts.
 *
 * PIC Command Words:
 * - ICW1: Initialization Command Word 1 - Starts initialization sequence
 * - ICW2: Initialization Command Word 2 - Sets the interrupt vector offset (where IRQs map to)
 * - ICW3: Initialization Command Word 3 - Configures master/slave cascade connection
 * - ICW4: Initialization Command Word 4 - Sets additional operating modes (8086 vs 8080 mode)
 * - OCW3: Operation Command Word 3      - Used to read PIC status registers (ISR/IRR)
 *
 * Reference: https://wiki.osdev.org/8259_PIC
 */

#include <stdint.h>

#include "include/io.h"
#include "include/pic.h"

/* 8259A PIC I/O port addresses */
#define PIC1_COMMAND 0x20  /* Master PIC command port */
#define PIC1_DATA    0x21  /* Master PIC data port */
#define PIC2_COMMAND 0xA0  /* Slave PIC command port */
#define PIC2_DATA    0xA1  /* Slave PIC data port */

/* PIC Initialization Command Words (ICWs) */
#define ICW1_ICW4      0x01  /* ICW4 needed */
#define ICW1_INIT      0x10  /* Initialization - required! */
#define ICW4_8086      0x01  /* 8086/88 (MCS-80/85) mode */

/* OCW3 (Operation Command Word 3) - for reading ISR/IRR */
#define PIC_READ_IRR   0x0A  /* OCW3: Read Interrupt Request Register (next CMD read) */
#define PIC_READ_ISR   0x0B  /* OCW3: Read In-Service Register (next CMD read) */

/* End-of-Interrupt (EOI) command code */
#define PIC_EOI        0x20

/**
 * Send End-of-Interrupt (EOI) command to the PIC(s).
 *
 * This must be sent to the PIC after handling an IRQ to signal that the interrupt has been
 * processed and the PIC can send more interrupts. For IRQs 8-15 (slave PIC), we must send
 * EOI to both the slave and master PICs because the slave is cascaded through the master.
 *
 * @param irq The IRQ number (0-15) that was just handled
 */
void pic_send_eoi(uint8_t irq) {
	/* IRQs 8-15 come from the slave PIC, which is connected to the master's IRQ 2.
	 * We must send EOI to both PICs in this case. */
	if (irq >= 8) {
		outb(PIC2_COMMAND, PIC_EOI);
	}
	/* Always send EOI to the master PIC (IRQs 0-7 come directly from master) */
	outb(PIC1_COMMAND, PIC_EOI);
}

/**
 * Remap the PICs to new vector offsets.
 *
 * By default, the BIOS configures the PICs to use vectors 0x08-0x0F (IRQs 0-7) and
 * 0x70-0x77 (IRQs 8-15). This conflicts with CPU exceptions (vectors 0-31), so we
 * remap them to higher vectors (typically 0x20-0x2F).
 *
 * The remapping process uses Initialization Command Words (ICWs) to reconfigure the PICs:
 * - ICW1: Start initialization sequence (in cascade mode)
 * - ICW2: Set vector offset (where IRQs are mapped in the interrupt vector table)
 * - ICW3: Configure master/slave relationship (which IRQ the slave is connected to)
 * - ICW4: Set additional configuration (8086 mode)
 *
 * After remapping, all IRQs are masked (disabled) by default. Drivers must explicitly
 * unmask their IRQs using pic_unmask() or register_irq(). This is the modern approach
 * used by Linux and other production kernels for safety and explicit control.
 *
 * @param master_offset Vector offset for master PIC (IRQs 0-7), typically 0x20
 * @param slave_offset  Vector offset for slave PIC (IRQs 8-15), typically 0x28
 */
void pic_remap(uint8_t master_offset, uint8_t slave_offset) {
	/* ICW1: Start initialization sequence in cascade mode.
	 * 0x11 = 0001 0001b
	 *        ││││ ││││
	 *        ││││ │││└─ ICW4 needed
	 *        ││││ ││└── Single mode (0 = cascade mode)
	 *        ││││ │└─── Call address interval (unused in x86)
	 *        ││││ └──── Level triggered mode (0 = edge triggered)
	 *        │││└────── Initialization bit (must be 1)
	 *        └┴┴─────── Not used in x86 mode */
	outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

	/* ICW2: Set vector offset.
	 * Master PIC (IRQs 0-7) will be mapped to master_offset..master_offset+7
	 * Slave PIC (IRQs 8-15) will be mapped to slave_offset..slave_offset+7 */
	outb(PIC1_DATA, master_offset);  /* Master PIC vector offset */
	outb(PIC2_DATA, slave_offset);   /* Slave PIC vector offset */

	/* ICW3: Configure master/slave connection.
	 * Master PIC: Tell it there's a slave PIC at IRQ2 (0000 0100b = bit 2 set)
	 * Slave PIC: Tell it its cascade identity (0000 0010b = IRQ 2) */
	outb(PIC1_DATA, 0x04);  /* Master: slave on IRQ2 */
	outb(PIC2_DATA, 0x02);  /* Slave: cascade identity (IRQ 2) */

	/* ICW4: Set additional modes.
	 * 0x01 = 8086/88 mode (as opposed to MCS-80/85 mode)
	 * This is required for x86 systems. */
	outb(PIC1_DATA, ICW4_8086);
	outb(PIC2_DATA, ICW4_8086);

	/* Mask all interrupts on both PICs.
	 * Modern approach: start with all IRQs disabled (masked), then drivers explicitly
	 * unmask their IRQs when ready. This prevents spurious interrupts and gives explicit
	 * control over which interrupts are active.
	 * 0xFF = 1111 1111b = all 8 IRQ lines masked (disabled) */
	outb(PIC1_DATA, 0xFF);  /* Mask all IRQs on master PIC (IRQs 0-7) */
	outb(PIC2_DATA, 0xFF);  /* Mask all IRQs on slave PIC (IRQs 8-15) */
}

/**
 * Unmask (enable) an individual IRQ line.
 *
 * By default, most IRQ lines are masked (disabled) after remapping. This function clears
 * the mask bit for a specific IRQ, allowing that interrupt to be delivered to the CPU.
 *
 * @param irq IRQ number to unmask (0-15)
 */
void pic_unmask(uint8_t irq) {
	uint16_t port;
	uint8_t value;

	/* Determine which PIC controls this IRQ */
	if (irq < 8) {
		port = PIC1_DATA;  /* IRQs 0-7: Master PIC */
	}
	else {
		port = PIC2_DATA;  /* IRQs 8-15: Slave PIC */
		irq -= 8;          /* Convert to slave PIC's local IRQ number (0-7) */
	}

	/* Read current mask, clear the bit for this IRQ (0 = unmasked), and write back.
	 * Each bit in the mask register corresponds to one IRQ line:
	 * - Bit 0 = IRQ 0/8, Bit 1 = IRQ 1/9, ..., Bit 7 = IRQ 7/15
	 * - 0 = IRQ enabled (unmasked), 1 = IRQ disabled (masked) */
	value = inb(port) & ~(1 << irq);
	outb(port, value);
}

/**
 * Mask (disable) an individual IRQ line.
 *
 * This function sets the mask bit for a specific IRQ, preventing that interrupt from
 * being delivered to the CPU. Useful for temporarily disabling an interrupt source.
 *
 * @param irq IRQ number to mask (0-15)
 */
void pic_mask(uint8_t irq) {
	uint16_t port;
	uint8_t value;

	/* Determine which PIC controls this IRQ */
	if (irq < 8) {
		port = PIC1_DATA;  /* IRQs 0-7: Master PIC */
	}
	else {
		port = PIC2_DATA;  /* IRQs 8-15: Slave PIC */
		irq -= 8;          /* Convert to slave PIC's local IRQ number (0-7) */
	}
	/* Read current mask, set the bit for this IRQ (1 = masked), and write back */
	value = inb(port) | (1 << irq);
	outb(port, value);
}

/**
 * Helper function to read PIC interrupt registers (IRR or ISR).
 *
 * The PIC has two internal 8-bit status registers that can be read via OCW3:
 * - IRR (Interrupt Request Register): Shows which IRQ lines are raised but not yet serviced
 * - ISR (In-Service Register): Shows which IRQ lines are currently being serviced
 *
 * This function sends an OCW3 command to both PICs and reads their status registers,
 * combining them into a 16-bit value.
 *
 * @param ocw3 OCW3 command: PIC_READ_IRR (0x0A) or PIC_READ_ISR (0x0B)
 * @return 16-bit value: [slave PIC (15:8)] [master PIC (7:0)]
 */
static uint16_t pic_get_irq_reg(uint8_t ocw3) {
	/* Send OCW3 command to both PICs to select which register to read.
	 * The PIC remembers this setting for future reads of the command port. */
	outb(PIC1_COMMAND, ocw3);
	outb(PIC2_COMMAND, ocw3);

	/* Read the selected register from both PICs.
	 * Master PIC returns bits 7:0 (IRQs 0-7)
	 * Slave PIC returns bits 15:8 (IRQs 8-15) */
	return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}

/**
 * Read the Interrupt Request Register (IRR) from both PICs.
 *
 * The IRR shows which IRQ lines have been raised but not yet sent to the CPU.
 * An IRQ appears in the IRR when:
 * - The hardware device raises the interrupt line
 * - The IRQ is not masked in the IMR (Interrupt Mask Register)
 *
 * Use cases:
 * - Debugging: Check if interrupts are being raised by hardware
 * - Spurious interrupt detection: Verify if IRQ bit is set when interrupt fires
 * - Hardware troubleshooting: See if device is signaling correctly
 *
 * @return 16-bit bitmask where bit N = 1 means IRQ N is pending
 */
uint16_t pic_get_irr(void) {
	return pic_get_irq_reg(PIC_READ_IRR);
}

/**
 * Read the In-Service Register (ISR) from both PICs.
 *
 * The ISR shows which IRQ lines are currently being serviced (sent to CPU but not yet EOI'd).
 * An IRQ appears in the ISR when:
 * - The PIC has sent the interrupt to the CPU
 * - The EOI (End of Interrupt) command has not yet been sent for this IRQ
 *
 * Use cases:
 * - Debugging: Verify EOI is being sent correctly (ISR should clear after EOI)
 * - Nested interrupt analysis: See which interrupts are active simultaneously
 * - Hang debugging: Check if an interrupt is stuck in ISR (missing EOI)
 *
 * Note: Bit 2 (IRQ 2) will be set whenever any slave PIC interrupt (IRQs 8-15) is active,
 * due to the cascade connection.
 *
 * @return 16-bit bitmask where bit N = 1 means IRQ N is being serviced
 */
uint16_t pic_get_isr(void) {
	return pic_get_irq_reg(PIC_READ_ISR);
}
