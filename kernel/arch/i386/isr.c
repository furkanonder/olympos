#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "include/interrupts.h"

/*
 * Exception message lookup table
 *
 * Maps CPU exception numbers (0-31) to human-readable error messages.
 * Used when no custom handler is registered for an exception.
 */
static const char* exception_messages[32] = {
	"Division By Zero",             /* 0 */
	"Debug",                        /* 1 */
	"Non Maskable Interrupt",       /* 2 */
	"Breakpoint",                   /* 3 */
	"Into Detected Overflow",       /* 4 */
	"Out of Bounds",                /* 5 */
	"Invalid Opcode",               /* 6 */
	"No Coprocessor",               /* 7 */
	"Double Fault",                 /* 8 */
	"Coprocessor Segment Overrun",  /* 9 */
	"Bad TSS",                      /* 10 */
	"Segment Not Present",          /* 11 */
	"Stack Fault",                  /* 12 */
	"General Protection Fault",     /* 13 */
	"Page Fault",                   /* 14 */
	"Unknown Interrupt",            /* 15 */
	"Coprocessor Fault",            /* 16 */
	"Alignment Check",              /* 17 */
	"Machine Check",                /* 18 */
	"Reserved",                     /* 19 */
	"Reserved",                     /* 20 */
	"Reserved",                     /* 21 */
	"Reserved",                     /* 22 */
	"Reserved",                     /* 23 */
	"Reserved",                     /* 24 */
	"Reserved",                     /* 25 */
	"Reserved",                     /* 26 */
	"Reserved",                     /* 27 */
	"Reserved",                     /* 28 */
	"Reserved",                     /* 29 */
	"Reserved",                     /* 30 */
	"Reserved"                      /* 31 */
};

/*
 * ISR handler registration table
 *
 * Array of function pointers for custom ISR handlers. NULL entries fall back to default panic behavior.
 */
static isr_handler_fn isr_handlers[32] = {0};

/**
 * Register a custom handler for an ISR
 *
 * Allows modules to register custom exception handlers instead of using
 * the default panic behavior. This enables proper exception handling
 * for things like page faults, general protection faults, etc.
 *
 * @param isr ISR number (0-31)
 * @param handler Function to call when this ISR occurs
 * @return 0 on success, -1 on invalid ISR number
 */
int register_isr(int isr, isr_handler_fn handler) {
	if (isr < 0 || isr >= 32) {
		return -1;
	}
	isr_handlers[isr] = handler;
	return 0;
}

/**
 * Main ISR handler dispatcher
 *
 * This function is called by the assembly ISR stubs with a pointer to
 * the saved CPU state. It either dispatches to a registered custom
 * handler or shows a panic message for unhandled exceptions.
 *
 * @param r Pointer to saved CPU register state
 */
void isr_handler(regs_t* r) {
	if (r->int_no >= 32) {
		panic("Invalid ISR number: %u\n", r->int_no);
		return;
	}
	if (isr_handlers[r->int_no] != NULL) {
		isr_handlers[r->int_no](r);
	}
	else {
		const char* reason = (r->int_no < 32) ? exception_messages[r->int_no] : "Unknown";
		panic("Exception %u: %s\n", r->int_no, reason);
	}
}
