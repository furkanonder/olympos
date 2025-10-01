/**
 * PS/2 Keyboard Driver (Intel 8042 Controller)
 *
 * This driver handles input from a standard PS/2 keyboard using the Intel 8042 keyboard
 * controller. The keyboard sends scancodes when keys are pressed (make codes) and released
 * (break codes), which must be translated to ASCII characters.
 *
 * Historical Context:
 * - The original IBM PC/XT (1981) defined Scancode Set 1
 * - The IBM PC/AT (1984) introduced the 8042 controller and Scancode Set 2
 * - Modern systems: keyboards use Set 2 internally, but the 8042 translates to Set 1
 *   for BIOS compatibility, which is why this driver uses Set 1
 *
 * This is a minimal implementation that:
 * - Handles basic printable characters (no shift/caps lock)
 * - Ignores break codes (key releases)
 * - Ignores extended keys (arrow keys, function keys beyond F10)
 * - Simply echoes characters to the terminal
 *
 * Reference: https://wiki.osdev.org/PS/2_Keyboard
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "../include/irq.h"
#include "../include/io.h"
#include "../include/interrupts.h"

/* Intel 8042 PS/2 Controller I/O Ports */
#define KBD_DATA_PORT   0x60  /* Data port - read scancodes from here */
#define KBD_STATUS_PORT 0x64  /* Status register - check for data availability */
#define KBD_STATUS_OBF  0x01  /* Output Buffer Full - bit 0: data ready to read */

/* Forward declarations */
static void keyboard_on_irq(void);

/**
 * IRQ trampoline for the keyboard driver.
 *
 * This function is registered with the IRQ system and called when IRQ 1 (keyboard) fires.
 * It forwards the call to the actual keyboard handler.
 *
 * @param r Saved CPU register state captured on interrupt entry. Unused here,
 *          the handler reads scancode(s) directly from the controller.
 */
static void kb_irq_trampoline(regs_t* r) {
	(void)r;
	keyboard_on_irq();
}

/**
 * IBM PC/XT Scancode Set 1 to ASCII translation table.
 *
 * This table maps hardware scancodes (make codes) from the keyboard to ASCII characters.
 * Scancode Set 1 was defined by the original IBM PC/XT (1981) and is still used today
 * because the 8042 controller translates modern keyboards to this format for compatibility.
 *
 * Layout:
 * - Index: Scancode value (0x00-0x7F are make codes)
 * - Value: ASCII character, or 0 for non-printable/modifier keys
 *
 * Make codes (key press): 0x00-0x7F
 * Break codes (key release): 0x80-0xFF (make code | 0x80)
 *
 * Limitations of this minimal table:
 * - No shift support (lowercase only)
 * - No caps lock handling
 * - Function keys F1-F10 mapped but unused (return 0)
 * - Extended keys (arrows, F11-F12) not included (require 0xE0 prefix)
 * - Control/Alt combinations not handled
 *
 * Complete scancode reference: https://wiki.osdev.org/PS/2_Keyboard#Scan_Code_Set_1
 */
static char scancode_to_ascii[128] = {
	/* 0x00-0x0F */
	0,      /* 0x00: (error) */
	27,     /* 0x01: ESC */
	'1','2','3','4','5','6','7','8','9','0','-','=',
	'\b',   /* 0x0E: Backspace */
	'\t',   /* 0x0F: Tab */

	/* 0x10-0x1F: QWERTY row + Enter + Ctrl */
	'q','w','e','r','t','y','u','i','o','p','[',']',
	'\n',   /* 0x1C: Enter */
	0,      /* 0x1D: Left Ctrl */
	'a','s',

	/* 0x20-0x2F: ASDF row + Shift */
	'd','f','g','h','j','k','l',';','\'','`',
	0,      /* 0x2A: Left Shift */
	'\\',

	/* 0x30-0x3F: ZXCV row + keypad */
	'z','x','c','v','b','n','m',',','.','/',
	0,      /* 0x36: Right Shift */
	'*',    /* 0x37: Keypad * */
	0,      /* 0x38: Left Alt */
	' ',    /* 0x39: Space */
	0,      /* 0x3A: Caps Lock */

	/* 0x3B-0x44: Function keys F1-F10 */
	0,0,0,0,0,0,0,0,0,0,

	/* 0x45-0x49: Num Lock, Scroll Lock, Keypad 7-9 */
	0,0,0,0,0,

	/* 0x4A-0x53: Keypad - , 4-6, +, 1-3, 0, Del */
	0,0,0,0,0,0,0,0
};

/**
 * Initialize the keyboard driver and register its IRQ handler.
 *
 * This function sets up the keyboard driver by registering the IRQ 1 handler.
 * The keyboard controller (8042) triggers IRQ 1 whenever a scancode is available
 * in the output buffer.
 *
 * Note: This assumes the BIOS has already initialized the 8042 controller in
 * translation mode (Scancode Set 2 → Set 1 translation enabled).
 */
void keyboard_initialize(void) {
	reqister_irq(1, kb_irq_trampoline);
	printf("[  OK  ] Keyboard driver initialized (IRQ 1).\n");
}

/**
 * Low-level keyboard IRQ handler.
 *
 * This handler is called whenever IRQ 1 fires (keyboard data available). It:
 * 1. Checks if data is available in the output buffer
 * 2. Reads the scancode from the data port (0x60)
 * 3. Filters out break codes (bit 7 set = key release)
 * 4. Translates make codes to ASCII using the scancode table
 * 5. Echoes printable characters to the terminal
 *
 * Scancode format:
 * - Make code (key press):   0x00-0x7F (bit 7 = 0)
 * - Break code (key release): 0x80-0xFF (bit 7 = 1, i.e., make code | 0x80)
 *
 * Extended keys (arrows, F11, etc.) send 0xE0 prefix followed by the scancode.
 * This minimal driver ignores extended keys.
 */
static void keyboard_on_irq(void) {
	/* Check if data is available in the output buffer.
	 * Bit 0 (OBF) of the status register indicates data is ready to read. */
	if ((inb(KBD_STATUS_PORT) & KBD_STATUS_OBF) == 0) {
		return;  /* No data available */
	}

	/* Read the scancode from the data port */
	uint8_t sc = inb(KBD_DATA_PORT);

	/* Filter out break codes (key releases).
	 * Break codes have bit 7 set (scancode | 0x80).
	 * Example: 'A' pressed = 0x1E, 'A' released = 0x9E */
	if (sc & 0x80) {
		return;  /* Ignore key releases */
	}

	/* Translate scancode to ASCII using the lookup table.
	 * If the scancode maps to 0, it's a non-printable key (Ctrl, Shift, etc.) */
	char c = (sc < 128) ? scancode_to_ascii[sc] : 0;

	/* Echo printable characters to the terminal */
	if (c) {
		putchar(c);
	}
}
