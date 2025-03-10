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
static uint8_t terminal_color;
/* Pointer to the VGA text buffer */
static uint16_t* terminal_buffer;

/**
 * Initializes the terminal interface
 *
 * Sets up the terminal with default colors (light grey on black)
 * and clears the screen by filling it with spaces.
 */
void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = VGA_MEMORY;
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
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
 * Puts a character at a specific position in the terminal
 *
 * @param c 	Character to display
 * @param color Color attribute for this character
 * @param x 	Column position (0 to VGA_WIDTH - 1)
 * @param y 	Row position (0 to VGA_HEIGHT - 1)
 */
void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

/**
 * Writes a character to the terminal at current cursor position
 *
 * Handles basic cursor advancement and wrapping:
 * - Processes '\n' as a newline (advances row, resets column)
 * - Moves to next line when reaching end of current line
 * - Wraps back to top when reaching bottom of screen
 *
 * @param c Character to write
 */
void terminal_putchar(char c) {
    unsigned char uc = c;
    if (uc == '\n') {
        terminal_row++;
        terminal_column = 0;
    }
    else {
        terminal_putentryat(uc, terminal_color, terminal_column, terminal_row);
        if (++terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) {
                terminal_row = 0;
            }
        }
    }
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
