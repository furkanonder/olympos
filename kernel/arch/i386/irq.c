#include <stddef.h>
#include <stdint.h>

#include "include/irq.h"
#include "include/interrupts.h"
#include "include/pic.h"

/* Simple fixed-size table for legacy PIC (16 lines) */
static irq_handler_fn irq_handlers[16] = {0};

/**
 * Low-level IRQ entry point called from assembly stubs.
 *
 * @param r Saved CPU register state captured on interrupt entry.
 *          The field r->int_no contains the vector number; hardware IRQs are mapped to 32..47 after PIC remap.
 *
 * Dispatches to any registered handler for the IRQ and sends EOI to the PIC.
 */
void irq_handler(regs_t* r) {
	uint32_t int_no = r->int_no;
	if (int_no >= 32 && int_no < 48) {
		int irq = (int)(int_no - 32);
		if (irq_handlers[irq]) {
			irq_handlers[irq](r);
		}
		pic_send_eoi((uint8_t)irq);
	}
}

/**
 * Register a handler and unmask the IRQ at the PIC.
 *
 * @param irq     Hardware IRQ line number (0..15 on legacy PIC)
 * @param handler Function to invoke when this IRQ fires
 * @return 0 on success, -1 if the IRQ is out of range
 */
int reqister_irq(int irq, irq_handler_fn handler) {
	if (irq < 0 || irq >= 16) {
		return -1;
	}
	irq_handlers[irq] = handler;
	pic_unmask((uint8_t)irq);
	return 0;
}

/**
 * Unregister a handler and mask the IRQ at the PIC.
 *
 * This function removes the registered handler for the specified IRQ and masks (disables)
 * the interrupt at the PIC level, preventing further interrupts from this line.
 *
 * @param irq Hardware IRQ line number (0..15 on legacy PIC)
 * @return 0 on success, -1 if the IRQ is out of range
 */
int unregister_irq(int irq) {
	if (irq < 0 || irq >= 16) {
		return -1;
	}
	pic_mask((uint8_t)irq);   /* Mask the IRQ first to prevent spurious interrupts */
	irq_handlers[irq] = NULL; /* Then remove the handler */
	return 0;
}
