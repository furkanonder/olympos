// Reference: https://wiki.osdev.org/Serial_Ports

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include <kernel/serial.h>

#include "../include/io.h"

/**
 * Initialize a serial port with specified baud rate
 *
 * @param port            Base port address (e.g., SERIAL_COM1_BASE)
 * @param baud_divisor    Baud rate divisor (e.g., SERIAL_BAUD_115200)
 * @return                0 if initialization successful, 1 otherwise
 */
int serial_setup(uint16_t port, uint16_t baud_divisor) {
    // Disable all interrupts
    outb(port + SERIAL_INTERRUPT_ENABLE_REG, 0x00);
    // Enable DLAB (set baud rate divisor)
    outb(port + SERIAL_LINE_CONTROL_REG, SERIAL_LINE_DLAB);
    // Set divisor (lo byte)
    outb(port + SERIAL_DIVISOR_LSB_REG, baud_divisor & 0xFF);
    // Set divisor (hi byte)
    outb(port + SERIAL_DIVISOR_MSB_REG, (baud_divisor >> 8) & 0xFF);
    // 8 bits, no parity, one stop bit
    outb(port + SERIAL_LINE_CONTROL_REG, SERIAL_LINE_8N1);
    // Enable FIFO, clear them, with 14-byte threshold
    outb(port + SERIAL_FIFO_CONTROL_REG, SERIAL_FIFO_ENABLE | SERIAL_FIFO_CLEAR_RECEIVE | SERIAL_FIFO_CLEAR_TRANSMIT
                                         | SERIAL_FIFO_14_BYTE_THRESHOLD);
    // IRQs enabled, RTS/DSR set
    outb(port + SERIAL_MODEM_CONTROL_REG, SERIAL_MODEM_DTR | SERIAL_MODEM_RTS | SERIAL_MODEM_OUT2);

    // Test serial chip (send byte 0xAE and check if serial returns same byte)
    outb(port + SERIAL_DATA_REG, 0xAE);
    // Check if serial is faulty (i.e: not the same byte as sent)
    if (inb(port + SERIAL_DATA_REG) != 0xAE) {
        return 1;
    }
    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(port + SERIAL_MODEM_CONTROL_REG, SERIAL_MODEM_DTR | SERIAL_MODEM_RTS | SERIAL_MODEM_OUT1
                                          | SERIAL_MODEM_OUT2);
    return 0;
}

/**
 * Check if the transmit buffer is empty and ready for new data
 *
 * @param port Base port address
 * @return true if transmit buffer is empty, false otherwise
 */
bool serial_is_transmit_empty(uint16_t port) {
    return inb(port + SERIAL_LINE_STATUS_REG) & SERIAL_LINE_STATUS_THRE;
}

/**
 * Send a single character over the serial port
 *
 * @param port Base port address
 * @param c Character to send
 */
void serial_write_char(uint16_t port, char c) {
    // Wait for the transmit buffer to be empty
    while (!serial_is_transmit_empty(port)) {
        // Busy wait
    }
    outb(port + SERIAL_DATA_REG, c);
}

/**
 * Send a string over the serial port
 *
 * @param port Base port address
 * @param str Null-terminated string to send
 */
void serial_write_string(uint16_t port, const char* str) {
    while (*str) {
        serial_write_char(port, *str);
        str++;
    }
}

/**
 * Check if data is available to read
 *
 * @param port Base port address
 * @return true if data is available, false otherwise
 */
bool serial_has_received(uint16_t port) {
    return inb(port + SERIAL_LINE_STATUS_REG) & SERIAL_LINE_STATUS_DR;
}

/**
 * Read a single character from the serial port
 *
 * @param port Base port address
 * @return Character read from the port
 */
char serial_read_char(uint16_t port) {
    // Wait for data to be available
    while (!serial_has_received(port)) {
        // Busy wait
    }
    return inb(port + SERIAL_DATA_REG);
}

/**
 * Enable serial port interrupts
 *
 * @param port Base port address
 */
void serial_enable_interrupts(uint16_t port) {
    // Enable received data available interrupt
    outb(port + SERIAL_INTERRUPT_ENABLE_REG, SERIAL_INT_ENABLE_RDA);
}

/**
 * Disable serial port interrupts
 *
 * @param port Base port address
 */
void serial_disable_interrupts(uint16_t port) {
    outb(port + SERIAL_INTERRUPT_ENABLE_REG, SERIAL_INT_DISABLE_ALL);
}

/**
 * Initialize serial port and print status messages
 *
 * @param port 			Base port address
 * @param baud_divisor	Baud rate divisor
 */
void serial_initialize(uint16_t port, uint16_t baud_divisor) {
    if (serial_setup(port, baud_divisor)) {
        printf("Serial port initialization successful!\n");
        printf("Serial port: 0x%x, Serial Baud Rate: %d\n", port, 115200 / baud_divisor);
        serial_write_string(port, "=======================================\n");
        serial_write_string(port, "Olympos Serial Debug Output\n");
        serial_write_string(port, "=======================================\n\n");
    }
    else {
        printf("Warning: Serial port initialization failed!\n");
    }
}
