#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H

#include <stddef.h>

/**
 * Initializes the terminal interface
 *
 * Sets up the terminal with default colors (light grey on black)
 * and clears the screen by initializing the VGA hardware.
 */
void terminal_initialize(void);

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
void terminal_putchar(char c);

/**
 * Writes a string of specific length to the terminal
 *
 * @param data Pointer to the string to write
 * @param size Number of characters to write
 */
void terminal_write(const char* data, size_t size);

/**
 * Writes a null-terminated string to the terminal
 *
 * @param data Pointer to the null-terminated string to write
 */
void terminal_writestring(const char* data);

/**
 * Handles backspace by moving cursor back and erasing character
 *
 * Moves the cursor one position back if not at the start of the line,
 * replaces the character with a space, and updates the cursor position.
 */
void terminal_backspace(void);

#endif
