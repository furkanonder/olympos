#ifndef ARCH_I386_PIC_H
#define ARCH_I386_PIC_H

#include <stdint.h>

/** Remap master/slave PICs to new vector offsets. */
void pic_remap(uint8_t master_offset, uint8_t slave_offset);

/** Send End-of-Interrupt to the appropriate PIC(s) for the given IRQ. */
void pic_send_eoi(uint8_t irq);

/** Unmask (enable) a specific IRQ line at the PIC. */
void pic_unmask(uint8_t irq);

/** Mask (disable) a specific IRQ line at the PIC. */
void pic_mask(uint8_t irq);

/** Read the Interrupt Request Register (IRR) from both PICs. Returns pending interrupts. */
uint16_t pic_get_irr(void);

/** Read the In-Service Register (ISR) from both PICs. Returns interrupts being serviced. */
uint16_t pic_get_isr(void);

#endif
