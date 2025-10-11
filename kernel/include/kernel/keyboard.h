#ifndef _KERNEL_KEYBOARD_H
#define _KERNEL_KEYBOARD_H

/**
 * Initializes the PS/2 keyboard driver
 *
 * Registers the keyboard IRQ handler (IRQ 1) to handle keyboard input events.
 * Must be called during kernel initialization before keyboard input is needed.
 */
void keyboard_initialize(void);

/**
 * Blocking keyboard character input
 *
 * Waits for and returns the next character from the keyboard input buffer. This function blocks (using HLT instruction)
 * until a character is available. Used by getchar() to implement blocking input for the shell.
 *
 * @return ASCII character code of the key pressed
 */
int keyboard_callback_getchar(void);

#endif
