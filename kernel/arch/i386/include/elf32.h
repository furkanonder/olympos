// Reference: https://web.mit.edu/freebsd/head/sys/sys/elf32.h

#ifndef ELF32_H
#define ELF32_H

#include <stdint.h>

typedef uint32_t	Elf32_Addr;
typedef uint32_t	Elf32_Off;
typedef uint16_t	Elf32_Half;
typedef uint32_t	Elf32_Word;
typedef uint8_t		Elf32_Char;

#define ELF32_ST_TYPE(info)		((info) & 0xf)
#define ELF_SYM_TYPE_FUNC		0x2

/*
 * Section header.
 */
struct Elf32_Shdr {
    Elf32_Word	sh_name;		/* Section name (index into the section header string table). */
    Elf32_Word	sh_type;		/* Section type. */
    Elf32_Word	sh_flags;		/* Section flags. */
    Elf32_Addr	sh_addr;		/* Address in memory image. */
    Elf32_Off	sh_offset;		/* Offset in file. */
    Elf32_Word	sh_size;		/* Size in bytes. */
    Elf32_Word	sh_link;		/* Index of a related section. */
    Elf32_Word	sh_info;		/* Depends on section type. */
    Elf32_Word	sh_addralign;	/* Alignment in bytes. */
    Elf32_Word	sh_entsize;		/* Size of each entry in section. */
} __attribute__((packed));
typedef struct Elf32_Shdr Elf32_Shdr_t;

/*
 * Symbol table entries.
 */
struct Elf32_Sym {
    Elf32_Word	    st_name;	/* String table index of name. */
    Elf32_Addr	    st_value;	/* Symbol value. */
    Elf32_Word	    st_size;	/* Size of associated object. */
    unsigned char	st_info;	/* Type and binding information. */
    unsigned char	st_other;	/* Reserved (not used). */
    Elf32_Half	    st_shndx;	/* Section index of symbol. */
} __attribute__((packed));
typedef struct Elf32_Sym Elf32_Sym_t;

#endif
