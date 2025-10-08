#ifndef _KERNEL_DEBUG_H
#define _KERNEL_DEBUG_H

#include <stdint.h>

#include <kernel/multiboot.h>

/**
 * Initialize debugging support with symbol information from multiboot
 *
 * @param mbi Pointer to multiboot information structure
 */
void debug_initialize(multiboot_info_t* mbi);

/**
 * Print a backtrace of the current call stack
 */
void print_backtrace(void);

/**
 * Find and return the symbol name for a given memory address
 *
 * @param addr The memory address to look up
 * @return The symbol name, or "unknown" if not found
 */
const char* find_symbol_for_address(uint32_t addr);

/**
 * End of all kernel ELF sections (page-aligned)
 * This marks where the kernel heap can begin
 */
extern uint32_t elf_sections_end;

#endif
