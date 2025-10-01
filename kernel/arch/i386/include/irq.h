#ifndef _KERNEL_IRQ_H
#define _KERNEL_IRQ_H

#include <stdint.h>

/* Forward-declare regs_t; actual definition is arch-specific */
typedef struct regs regs_t;

/**
 * A function pointer type for IRQ handlers.
 *
 * Handlers receive the saved CPU register state for the interrupt
 * and should return quickly. Long-running work should be deferred
 * to a later context when possible.
 */
typedef void (*irq_handler_fn)(regs_t*);

/**
 * Register an interrupt handler for the given IRQ line.
 *
 * @param irq     Hardware IRQ number (0..15 on legacy PIC)
 * @param handler Function to invoke when the IRQ fires
 * @return 0 on success, negative on error
 */
int reqister_irq(int irq, irq_handler_fn handler);

/**
 * Unregister an interrupt handler for the given IRQ line.
 *
 * @param irq Hardware IRQ number (0..15 on legacy PIC)
 * @return 0 on success, negative on error
 */
int unregister_irq(int irq);

#endif
