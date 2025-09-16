#ifndef KERNEL_INTERRUPTS_H
#define KERNEL_INTERRUPTS_H

#include <stdint.h>

/* Include arch-specific header for regs_t structure definition */
#include "../../arch/i386/include/interrupts.h"

/* Cross-arch IDT function (arch-specific types live under arch/<arch>/include) */
void idt_init(void);

#endif
