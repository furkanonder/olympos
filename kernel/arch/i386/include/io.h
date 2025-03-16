#ifndef ARCH_I386_IO_H
#define ARCH_I386_IO_H

/** outb:
 * Sends the given data to the given I/O port. Defined in io.nasm
 *
 * @param port    The I/O port to send the data to
 * @param data    The data to send to the I/O port
*/
void outb(unsigned short port, unsigned char data);

/** inb:
 * Read a byte from an I/O port. Defined in io.nasm
 *
 * @param port    The address of the I/O port
 * @return        The read byte
*/
unsigned char inb(unsigned short port);

#endif