#include <stdint.h>
#include <stddef.h>

#include "../include/vga.h"

/* Pointer to the VGA text buffer */
static uint16_t* terminal_buffer;
/* Extern from tty.c */
extern uint8_t terminal_color;

/**
 * Initialize VGA hardware
 *
 * Sets up the VGA text mode display:
 * - Clears the screen with light grey on black
 * - Resets cursor to top-left position (0, 0)
 */
void vga_initialize(void) {
    terminal_buffer = VGA_MEMORY;
    // Clear the screen
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        terminal_buffer[i] = vga_entry(' ', terminal_color);
    }
    vga_update_cursor(0);
}

/**
 * Write a character at a specific position on screen
 *
 * @param c     Character to write
 * @param color Combined foreground/background color attribute
 * @param x     Column position (0 to VGA_WIDTH - 1)
 * @param y     Row position (0 to VGA_HEIGHT - 1)
 */
void vga_write_char_at(unsigned char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

/**
 * Update the cursor to a specific position
 *
 * @param pos New cursor position (row * VGA_WIDTH + column)
 */
void vga_update_cursor_position(uint16_t pos) {
    vga_update_cursor(pos);
}

/**
 * Scroll the screen content up by one line
 *
 * Moves all lines up one position and clears the bottom line.
 * Used when terminal output reaches the bottom of the screen.
 */
void vga_scroll(void) {
    // Move all lines up one position
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t src_index = y * VGA_WIDTH + x;
            const size_t dst_index = (y - 1) * VGA_WIDTH + x;
            terminal_buffer[dst_index] = terminal_buffer[src_index];
        }
    }
    // Clear the last line
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
}
