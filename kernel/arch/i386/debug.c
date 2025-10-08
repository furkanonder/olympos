#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#include <kernel/multiboot.h>
#include <kernel/debug.h>

#include "include/elf32.h"

/* Global symbol information */
static Elf32_Sym_t* symbol_table = NULL;
static size_t symbol_count = 0;
static const char* string_table = NULL;
static size_t string_table_size = 0;
static int debug_initialized = 0;

/* Extern symbol from linker script marking end of kernel sections */
extern uint32_t _kernel_sections_end;

/* For marking the end of all sections. Kernel heap begins above this. */
uint32_t elf_sections_end = 0;

/**
 * Find and return the symbol name for a given memory address
 *
 * @param addr The memory address to look up
 * @return The symbol name, or "unknown" if not found
 */
const char* find_symbol_for_address(uint32_t addr) {
    if (!debug_initialized || !symbol_table || !string_table) {
        return "unknown (no symbols)";
    }

    /* Search through all symbols for a matching function */
    for (size_t i = 0; i < symbol_count; ++i) {
        unsigned char type = ELF32_ST_TYPE(symbol_table[i].st_info);
        if (type == ELF_SYM_TYPE_FUNC) {
            /* Calculate the function's address range */
            uint32_t sym_start = symbol_table[i].st_value;
            uint32_t sym_size = symbol_table[i].st_size;
            /* Check if our address is within this function's range */
            if (addr >= sym_start && addr <= sym_start + sym_size) {
                return string_table + symbol_table[i].st_name;
            }
        }
    }

    return "unknown";
}

/**
 * Get the base address of the function containing the given address
 *
 * @param addr The address to find the function for
 * @return The function's base address, or 0 if not found
 */
static uint32_t get_function_base_address(uint32_t addr) {
    if (!debug_initialized || !symbol_table || !string_table) {
        return 0;
    }

    /* Search through all symbols for a matching function */
    for (size_t i = 0; i < symbol_count; ++i) {
        /* Only consider symbols that are functions */
        unsigned char type = ELF32_ST_TYPE(symbol_table[i].st_info);
        if (type == ELF_SYM_TYPE_FUNC) {
            /* Calculate the function's address range */
            uint32_t sym_start = symbol_table[i].st_value;
            uint32_t sym_size = symbol_table[i].st_size;
            /* Check if our address is within this function's range */
            if (addr >= sym_start && addr <= sym_start + sym_size) {
                return sym_start;
            }
        }
    }

    return 0;
}

/**
 * Print a backtrace of the current call stack
 */
void print_backtrace(void) {
    /* Get the current frame pointer */
    uint32_t* frame_ptr;
    asm volatile ("movl %%ebp, %0" : "=r" (frame_ptr));

    printf("Stack backtrace:\n");
    uint32_t frame_count = 0;

    /* Track addresses to detect cycles */
    uint32_t prev_return_addr = 0;
    /* Reasonable limit to prevent infinite loops */
    uint32_t max_frames = 32;

    while (frame_ptr != NULL && frame_count < max_frames) {
        /* Get return address from stack frame */
        uint32_t return_addr = *(frame_ptr + 1);

        /* Check for repeated addresses indicating a cycle */
        if (return_addr == prev_return_addr) {
            printf("  [!] Cyclic backtrace detected\n");
            break;
        }
        prev_return_addr = return_addr;

        /* Look up function information */
        const char* func_name = find_symbol_for_address(return_addr);
        uint32_t func_base = get_function_base_address(return_addr);

        /* Format the output differently based on whether we found a symbol */
        if (func_base != 0) {
            uint32_t offset = return_addr - func_base;
            printf("  [%d] %s+0x%x (%p)\n", frame_count, func_name, offset, return_addr);
        }
        else {
            printf("  [%d] %s (%p)\n", frame_count, func_name, return_addr);
        }

        /* Move to previous frame */
        frame_ptr = (uint32_t*) *frame_ptr;
        frame_count++;
    }

    if (frame_count == 0) {
        printf("[FAILED] print_backtrace: No stack frames found\n");
    }
    else if (frame_count >= max_frames) {
        printf("[FAILED] print_backtrace: Maximum backtrace depth reached\n");
    }
}

/**
 * Find a specific section in the ELF section header table
 *
 * @param sht The section header table
 * @param sht_len Number of entries in the section header table
 * @param sh_names String table for section names
 * @param name Name of the section to find
 * @return Pointer to the section header, or NULL if not found
 */
static Elf32_Shdr_t* find_section(Elf32_Shdr_t *sht, size_t sht_len, const char *sh_names, const char *name) {
    for (size_t i = 0; i < sht_len; i++) {
        const char* section_name = sh_names + sht[i].sh_name;
        if (strcmp(section_name, name) == 0) {
            return &sht[i];
        }
    }
    return NULL;
}

/**
 * Initialize debugging support by extracting symbol information from multiboot.
 *
 * @param mbi Pointer to multiboot information structure
 */
void debug_initialize(multiboot_info_t *mbi) {
    /* Check if multiboot has ELF section information */
    if (!(mbi->flags & MULTIBOOT_INFO_ELF_SHDR)) {
        printf("[FAILED] No ELF section information available\n");
        return;
    }

    /* Access the ELF section header table */
    Elf32_Shdr_t* sht = (Elf32_Shdr_t*) mbi->u.elf_sec.addr;
    size_t sht_len = mbi->u.elf_sec.num;

    /* Make sure the section header string index is valid */
    if (mbi->u.elf_sec.shndx >= sht_len) {
        printf("[FAILED] debug_initialize: Invalid section header string index\n");
        return;
    }

    /* Get the section header string table */
    const char* sh_names = (const char*) sht[mbi->u.elf_sec.shndx].sh_addr;

    /* Find the symbol table */
    Elf32_Shdr_t* symtab_hdr = find_section(sht, sht_len, sh_names, ".symtab");
    if (symtab_hdr) {
        symbol_table = (Elf32_Sym_t*) symtab_hdr->sh_addr;
        symbol_count = symtab_hdr->sh_size / sizeof(Elf32_Sym_t);
    }
    else {
        printf("[FAILED] debug_initialize: Symbol table not found\n");
    }

    /* Find the string table */
    Elf32_Shdr_t* strtab_hdr = find_section(sht, sht_len, sh_names, ".strtab");
    if (strtab_hdr) {
        string_table = (const char*) strtab_hdr->sh_addr;
        string_table_size = strtab_hdr->sh_size;
    }
    else {
        printf("[FAILED] debug_initialize: String table not found\n");
    }

    /* Calculate the end of all ELF sections (for kernel heap) */
    uint32_t max_addr = (uint32_t) &_kernel_sections_end;
    for (size_t i = 0; i < sht_len; i++) {
        uint32_t section_end = sht[i].sh_addr + sht[i].sh_size;
        if (section_end > max_addr) {
            max_addr = section_end;
        }
    }
    /* Round up to page boundary (4 KiB) */
    elf_sections_end = (max_addr + 0xFFF) & ~0xFFF;

    /* Set initialization flag if we found both tables */
    if (symbol_table && string_table) {
        debug_initialized = 1;
        printf("[INFO] Symbol tables initialized (%zu symbols available)\n", symbol_count);
        printf("[INFO] Kernel sections end at %p\n", elf_sections_end);
    }
    else {
        printf("[FAILED] debug_initialize: Symbol information incomplete (symtab: %p, strtab: %p)\n",
				symbol_table, string_table);
    }
}
