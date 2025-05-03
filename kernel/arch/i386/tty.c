#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/tty.h>

#include "include/vga.h"

/* Current cursor position - row */
static size_t terminal_row;
/* Current cursor position - column */
static size_t terminal_column;
/* Current text color and background color */
uint8_t terminal_color;

/**
 * Initializes the terminal interface
 *
 * Sets up the terminal with default colors (light grey on black)
 * and clears the screen by initializing the VGA hardware.
 */
void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    // Initialize VGA hardware
    vga_initialize();
}

/**
 * Sets the terminal color
 *
 * @param color Combined color attribute byte (foreground and background)
 */
void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

/**
 * Check if terminal needs to scroll and handle it
 */
void terminal_check_scroll() {
    if (terminal_row == VGA_HEIGHT) {
        vga_scroll();
        terminal_row = VGA_HEIGHT - 1;
    }
}

/**
 * Handles a newline character by moving to the next line
 * and handling scrolling if needed
 */
void terminal_handle_newline() {
    terminal_column = 0;
    terminal_row++;
    terminal_check_scroll();
}

/**
 * Writes a character to the terminal at current cursor position
 *
 * Handles basic cursor advancement, wrapping, and scrolling:
 * - Processes '\n' as a newline (advances row, resets column)
 * - Moves to next line when reaching end of current line
 * - Scrolls the screen when reaching bottom
 * - Updates the hardware cursor position after writing
 *
 * @param c Character to write
 */
void terminal_putchar(char c) {
    unsigned char uc = c;
    if (uc == '\n') {
        terminal_handle_newline();
    }
    else {
        vga_write_char_at(uc, terminal_color, terminal_column, terminal_row);
        if (++terminal_column == VGA_WIDTH) {
            terminal_handle_newline();
        }
    }
    // Update hardware cursor
    uint16_t cursor_pos = terminal_row * VGA_WIDTH + terminal_column;
    vga_update_cursor_position(cursor_pos);
}

/**
 * Writes a string of specific length to the terminal
 *
 * @param data Pointer to the string to write
 * @param size Number of characters to write
 */
void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

/**
 * Writes a null-terminated string to the terminal
 *
 * @param data Pointer to the null-terminated string to write
 */
void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}
