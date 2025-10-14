#ifndef KERNEL_GDT_H
#define KERNEL_GDT_H

/* Include architecture-specific header for GDT structure definitions */
#include "../../arch/i386/include/gdt.h"

/* Cross-architecture GDT functions (architecture-specific types live under arch/<arch>/include) */
void gdt_init(void);

#endif
