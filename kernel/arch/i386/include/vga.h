#ifndef ARCH_I386_VGA_H
#define ARCH_I386_VGA_H

#include <stdint.h>
#include <stddef.h>

#include "io.h"

/* Standard VGA text mode dimensions */
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

/* VGA ports for controlling the text-mode cursor */
// CRT Controller Index Register - selects which register to write to
#define VGA_COMMAND_PORT    0x3D4
// CRT Controller Data Register - writes data to the selected register
#define VGA_DATA_PORT       0x3D5

/* VGA cursor position register commands */
// Cursor Location High Register (register 14) - stores the high byte of cursor position
#define VGA_HIGH_BYTE_COMMAND    0x0E
// Cursor Location Low Register (register 15) - stores the low byte of cursor position
#define VGA_LOW_BYTE_COMMAND     0x0F

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
 * Bit:     |  7 6 5 4  |  3 2 1 0  |
 * Content: |  BG color |  FG color |
 */
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

/**
 * Creates a VGA character entry
 *
 * @param uc    ASCII character to display
 * @param color Color attribute from vga_entry_color
 * @return      16-bit VGA character entry combining character and colors
 *
 * Bit layout:
 * Bit:     | 15 14 13 12 | 11 10 9 8 | 7 6 5 4 3 2 1 0 |
 * Content: |      BG     |     FG    | ASCII Character |
 */
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

/**
 * Updates the cursor position on the screen
 *
 * @param pos The 16-bit (row * VGA_WIDTH + column) position to update the cursor.
 */
static inline void vga_update_cursor(uint16_t pos) {
    /* Moving the cursor of the vga is done via two different I/O ports.
     * The cursor's position is determined with a 16 bits integer:
     *     0 means row zero, column zero;
     *     1 means row zero, column one;
     *     80 means row one, column zero and so on.
     *
     * Since the position is 16 bits large, and the out assembly code instruction argument is 8 bits,
     * the position must be sent in two turns, first 8 bits then the next 8 bits.
     *
     * The VGA has two I/O ports, one for accepting the data, and one for describing the data being received.
     * VGA_COMMAND_PORT is the port that describes the data and port VGA_DATA_PORT is for the data itself.
    */
    outb(VGA_COMMAND_PORT, VGA_HIGH_BYTE_COMMAND);
    outb(VGA_DATA_PORT, (uint8_t) (pos >> 8) & 0xFF);
    outb(VGA_COMMAND_PORT, VGA_LOW_BYTE_COMMAND);
    outb(VGA_DATA_PORT, (uint8_t) (pos & 0xFF));
}

/* VGA driver functions */

/**
 * Initialize VGA hardware
 *
 * Sets up the VGA text mode display:
 * - Clears the screen with light grey on black
 * - Resets cursor to top-left position (0, 0)
 */
void vga_initialize(void);

/**
 * Write a character at a specific position on screen
 *
 * @param c     Character to write
 * @param color Combined foreground/background color attribute
 * @param x     Column position (0 to VGA_WIDTH - 1)
 * @param y     Row position (0 to VGA_HEIGHT - 1)
 */
void vga_write_char_at(unsigned char c, uint8_t color, size_t x, size_t y);

/**
 * Update the cursor to a specific position
 *
 * @param pos New cursor position (row * VGA_WIDTH + column)
 */
void vga_update_cursor_position(uint16_t pos);

/**
 * Scroll the screen content up by one line
 *
 * Moves all lines up one position and clears the bottom line.
 * Used when terminal output reaches the bottom of the screen.
 */
void vga_scroll(void);

#endif
