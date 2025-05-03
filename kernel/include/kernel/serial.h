#ifndef _KERNEL_SERIAL_H
#define _KERNEL_SERIAL_H

#include <stdint.h>
#include <stdbool.h>

/* Serial port base addresses */
#define SERIAL_COM1_BASE                0x3F8	   /* COM1 base port */
#define SERIAL_COM2_BASE  				0x2F8	   /* COM2 base port */
#define SERIAL_COM3_BASE  				0x3E8	   /* COM3 base port */
#define SERIAL_COM4_BASE  				0x2E8      /* COM4 base port */
#define SERIAL_COM5_BASE  				0x5F8      /* COM5 base port */
#define SERIAL_COM6_BASE  				0x4F8      /* COM6 base port */
#define SERIAL_COM7_BASE  				0x5E8      /* COM7 base port */
#define SERIAL_COM8_BASE  				0x4E8      /* COM8 base port */

/* Serial port register offsets */
#define SERIAL_DATA_REG                 0x0        /* Data register (read/write) */
#define SERIAL_INTERRUPT_ENABLE_REG     0x1        /* Interrupt enable register */
#define SERIAL_DIVISOR_LSB_REG          0x0        /* Divisor LSB (when DLAB = 1) */
#define SERIAL_DIVISOR_MSB_REG          0x1        /* Divisor MSB (when DLAB = 1) */
#define SERIAL_FIFO_CONTROL_REG         0x2        /* FIFO control register (write) */
#define SERIAL_LINE_CONTROL_REG         0x3        /* Line control register */
#define SERIAL_MODEM_CONTROL_REG        0x4        /* Modem control register */
#define SERIAL_LINE_STATUS_REG          0x5        /* Line status register */

/* Line Control Register bits */
/*
 * Line Control Register - Configures data format and access to divisor
 *
 * Bit | Name    | Description
 * ----|---------|------------
 * 0-1 | WLS     | Word Length Select (00=5, 01=6, 10=7, 11=8 bits)
 *  2  | STB     | Stop Bits (0=1 stop bit, 1=2 stop bits)
 *  3  | PEN     | Parity Enable (0=disabled, 1=enabled)
 *  4  | EPS     | Even Parity Select (0=odd, 1=even)
 *  5  | SP      | Stick Parity
 *  6  | SB      | Set Break
 *  7  | DLAB    | Divisor Latch Access Bit
 */

/*
 * DLAB (Divisor Latch Access Bit) - Must be set to access divisor registers
 * Bit:   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * Value: | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | = 0x80
 */
#define SERIAL_LINE_DLAB                0x80       /* Divisor latch access bit */

/*
 * 8N1 Configuration: 8 data bits, no parity, 1 stop bit
 * Bit:   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * Value: | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | = 0x03
 *
 * Bits 0-1: 11 = 8 data bits
 * Bit 2:    0  = 1 stop bit
 * Bit 3:    0  = Parity disabled
 * Bits 4-5: 00 = No parity
 * Bit 6:    0  = Break control disabled
 * Bit 7:    0  = DLAB disabled
 */
#define SERIAL_LINE_8N1                 0x03       /* 8 bits, no parity, 1 stop bit */

/* Line Status Register bits */
/*
 * Line Status Register - Each bit indicates a specific UART condition
 *
 * Bit | Name | Description
 * ----|------|------------
 *  0  | DR   | Data Ready - Data received and ready to read
 *  1  | OE   | Overrun Error - Data lost due to buffer overflow
 *  2  | PE   | Parity Error - Received character has wrong parity
 *  3  | FE   | Framing Error - No valid stop bit detected
 *  4  | BI   | Break Indicator - Break condition detected
 *  5  | THRE | Transmitter Holding Register Empty
 *  6  | TEMT | Transmitter Empty - Transmitter completely idle
 *  7  | ERFI | Error in FIFO - Error(s) in FIFO buffer
 */

/*
 * Data Ready (DR) - Set when data has been received
 * Bit:   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * Value: | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | = 0x01
 */
#define SERIAL_LINE_STATUS_DR           0x01       /* Data ready */

/*
 * Transmitter Holding Register Empty (THRE)
 * Bit:   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * Value: | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | = 0x20
 */
#define SERIAL_LINE_STATUS_THRE         0x20       /* Transmitter holding register empty */

/* FIFO Control Register bits */
/*
 * FIFO Control Register - Configures FIFO buffers
 *
 * Bit | Name    | Description
 * ----|---------|------------
 *  0  | FIFOE   | FIFO Enable
 *  1  | RFR     | Receive FIFO Reset
 *  2  | XFR     | Transmit FIFO Reset
 *  3  | DMS     | DMA Mode Select
 * 4-5 | Reserved | Reserved
 * 6-7 | RTL     | Receive Trigger Level (00=1, 01=4, 10=8, 11=14 bytes)
 */

/*
 * Enable FIFOs
 * Bit:   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * Value: | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | = 0x01
 */
#define SERIAL_FIFO_ENABLE              0x01       /* Enable FIFOs */

/*
 * Clear Receive FIFO
 * Bit:   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * Value: | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | = 0x02
 */
#define SERIAL_FIFO_CLEAR_RECEIVE       0x02       /* Clear receive FIFO */

/*
 * Clear Transmit FIFO
 * Bit:   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * Value: | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | = 0x04
 */
#define SERIAL_FIFO_CLEAR_TRANSMIT      0x04       /* Clear transmit FIFO */

/*
 * 14-byte FIFO threshold
 * Bit:   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * Value: | 1 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | = 0xC0
 *
 * Bits 6-7: 11 = 14-byte trigger level
 */
#define SERIAL_FIFO_14_BYTE_THRESHOLD   0xC0       /* 14-byte FIFO threshold */

/* Modem Control Register bits */
/*
 * Modem Control Register - Controls modem interface signals
 *
 * Bit | Name | Description
 * ----|------|------------
 *  0  | DTR  | Data Terminal Ready
 *  1  | RTS  | Request To Send
 *  2  | OUT1 | Output 1 (user-designated)
 *  3  | OUT2 | Output 2 (enables UART interrupts)
 *  4  | LOOP | Loopback mode
 * 5-7 |      | Reserved
 */

/*
 * Data Terminal Ready (DTR)
 * Bit:   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * Value: | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | = 0x01
 */
#define SERIAL_MODEM_DTR                0x01       /* Data terminal ready */

/*
 * Request To Send (RTS)
 * Bit:   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * Value: | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | = 0x02
 */
#define SERIAL_MODEM_RTS                0x02       /* Request to send */

/*
 * Output 1 (OUT1)
 * Bit:   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * Value: | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | = 0x04
 */
#define SERIAL_MODEM_OUT1               0x04       /* Output 1 */

/*
 * Output 2 (OUT2) - Must be set to enable interrupts
 * Bit:   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * Value: | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | = 0x08
 */
#define SERIAL_MODEM_OUT2               0x08       /* Output 2 (enable IRQ) */

/* Interrupt Enable Register bits */
/*
 * Interrupt Enable Register - Enables specific interrupts
 *
 * Bit | Name | Description
 * ----|------|------------
 *  0  | ERBFI| Enable Received Data Available Interrupt
 *  1  | ETBEI| Enable Transmitter Holding Register Empty Interrupt
 *  2  | ELSI | Enable Receiver Line Status Interrupt
 *  3  | EDSSI| Enable Modem Status Interrupt
 * 4-7 |      | Reserved
 */

/*
 * Enable Received Data Available Interrupt
 * Bit:   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * Value: | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | = 0x01
 */
#define SERIAL_INT_ENABLE_RDA           0x01       /* Received data available */

/*
 * Disable All Interrupts
 * Bit:   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * Value: | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | = 0x00
 */
#define SERIAL_INT_DISABLE_ALL          0x00       /* Disable all interrupts */

/* Standard baud rate divisors */
/*
 * Baud rate calculation: divisor = 115200 / desired_baud_rate
 * The base clock for standard PC serial ports is 1.8432 MHz
 * With a divisor of 16, this gives us our base baud rate of 115200
 *
 * Divisor | Baud Rate
 * --------|----------
 *    1    | 115200
 *    2    | 57600
 *    3    | 38400
 *    6    | 19200
 *   12    | 9600
 */
#define SERIAL_BAUD_115200              1          /* Divisor for 115200 baud */
#define SERIAL_BAUD_57600               2          /* Divisor for 57600 baud */
#define SERIAL_BAUD_38400               3          /* Divisor for 38400 baud */
#define SERIAL_BAUD_19200               6          /* Divisor for 19200 baud */
#define SERIAL_BAUD_9600                12         /* Divisor for 115200 baud */

/**
 * Initialize serial port hardware with specified baud rate
 *
 * @param port Base port address (e.g., SERIAL_COM1_BASE)
 * @param baud_divisor Baud rate divisor (e.g., SERIAL_BAUD_115200)
 * @return 0 if initialization successful, 1 otherwise
 */
int serial_setup(uint16_t port, uint16_t baud_divisor);

/**
 * Initialize serial port and print status messages
 *
 * @param port Base port address (e.g., SERIAL_COM1_BASE)
 * @param baud_divisor Baud rate divisor (e.g., SERIAL_BAUD_115200)
 */
void serial_initialize(uint16_t port, uint16_t baud_divisor);

/**
 * Check if the transmit buffer is empty and ready for new data
 *
 * @param port Base port address
 * @return true if transmit buffer is empty, false otherwise
 */
bool serial_is_transmit_empty(uint16_t port);

/**
 * Send a single character over the serial port
 *
 * @param port Base port address
 * @param c Character to send
 */
void serial_write_char(uint16_t port, char c);

/**
 * Send a string over the serial port
 *
 * @param port Base port address
 * @param str Null-terminated string to send
 */
void serial_write_string(uint16_t port, const char* str);

/**
 * Send formatted output to the serial port (similar to printf)
 *
 * @param port Base port address
 * @param format Format string
 * @param ... Variable arguments
 * @return Number of characters written
 */
int serial_printf(uint16_t port, const char* format, ...);

/**
 * Check if data is available to read
 *
 * @param port Base port address
 * @return true if data is available, false otherwise
 */
bool serial_has_received(uint16_t port);

/**
 * Read a single character from the serial port
 *
 * @param port Base port address
 * @return Character read from the port
 */
char serial_read_char(uint16_t port);

/**
 * Enable serial port interrupts
 *
 * @param port Base port address
 */
void serial_enable_interrupts(uint16_t port);

/**
 * Disable serial port interrupts
 *
 * @param port Base port address
 */
void serial_disable_interrupts(uint16_t port);

#endif
