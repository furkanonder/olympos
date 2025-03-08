#ifndef ARCH_I386_VGA_H
#define ARCH_I386_VGA_H

#include <stdint.h>
#include <stddef.h>

/* Standard VGA text mode dimensions */
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

/**
 * VGA hardware text buffer address
 * Memory-mapped I/O location for VGA text mode
 */
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

/**
 * VGA color codes
 * Standard 16-color palette used in VGA text mode
 */
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

/**
 * Creates a VGA color attribute byte
 *
 * @param fg    Foreground color (text color)
 * @param bg    Background color
 * @return      Combined color attribute byte
 *
 * Bit layout:
 * Bit:     | 7 6 5 4  | 3 2 1 0 |
 * Content: | BG color | FG color|
 */
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

/**
 * Creates a VGA character entry
 *
 * @param uc    ASCII character to display
 * @param color Color attribute from vga_entry_color
 * @return		16-bit VGA character entry combining character and colors
 *
 * Bit layout:
 * Bit:     | 15 14 13 12 | 11 10 9 8 | 7 6 5 4 3 2 1 0 |
 * Content: |      BG     |     FG    | ASCII Character |
 */
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

#endif
