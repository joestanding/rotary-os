#include <rotary/serial.h>

/* ========================================================================= */

uint8 serial_initialised = 0;

/* ========================================================================= */

void serial_init() {

    // The Interrupt Enable Register is used to enable or disable various serial interrupts.
    // We set the register to zero to disable all interrupts.
    io_port_out(IO_PORT_SERIAL_COM1 + UART_IER, 0x00);

    // We set bit 7 of the Line Control Register to enable the Divisor Latch Access Bit.
    // When DLAB is set to '0' or '1' a number of registers are changed. It is necessary
    // as UART has 12 registers (including the work register) while using only 8 port addresses.
    io_port_out(IO_PORT_SERIAL_COM1 + UART_LCR, UART_LCR_DLAB);

    // Now DLAB is enabled, we can configure the baud rate.
    io_port_out(IO_PORT_SERIAL_COM1 + 0, UART_FCR_BAUD_38400_LOW);
    io_port_out(IO_PORT_SERIAL_COM1 + 1, UART_FCR_BAUD_38400_HIGH);

    // The Line Control Register sets the general connection parameters. 
    io_port_out(IO_PORT_SERIAL_COM1 + UART_LCR, UART_LCR_WORD_LENGTH_8B |
                                                UART_LCR_STOP_BIT_1 |
                                                UART_LCR_PARITY_NONE |
                                                UART_LCR_BREAK_DISABLE);

    // The FIFO Control Register is used to configure FIFO buffers.
    io_port_out(IO_PORT_SERIAL_COM1 + UART_FCR, UART_FCR_FIFO_ENABLE |
                                                UART_FCR_FIFO64_DISABLE |
                                                UART_FCR_CLEAR_RX_FIFO |
                                                UART_FCR_CLEAR_TX_FIFO |
                                                UART_FCR_INT_TRIG_14B);

    // Enable Force Data Terminal Ready, Force Request to Send
    io_port_out(IO_PORT_SERIAL_COM1 + UART_MCR, UART_MCR_FORCE_DTR |
                                                UART_MCR_FORCE_RTS |
                                                UART_MCR_AUX_OUTPUT_2);

    // Set in loopback mode, test the serial chip
    io_port_out(IO_PORT_SERIAL_COM1 + UART_MCR, UART_MCR_FORCE_RTS |
                                                UART_MCR_AUX_OUTPUT_1 |
                                                UART_MCR_AUX_OUTPUT_2 |
                                                UART_MCR_LOOPBACK_MODE);

    // Test that the serial port is in working order
    io_port_out(IO_PORT_SERIAL_COM1 + 0, 0xAE); // Test serial chip (send byte 0xAE and check if serial returns same byte)
    if(io_port_in(IO_PORT_SERIAL_COM1) != 0xAE) {
        return;
    } 

    // Put the serial port into normal operation by disabling loopback, enabling IRQs,
    // and enabling outputs 1 and 2.
    io_port_out(IO_PORT_SERIAL_COM1 + UART_MCR, UART_MCR_FORCE_DTR |
                                                UART_MCR_FORCE_RTS |
                                                UART_MCR_AUX_OUTPUT_1 |
                                                UART_MCR_AUX_OUTPUT_2);

    // We're all good!
    serial_initialised = 1;
}

/* ========================================================================= */

int serial_is_transmit_empty() {
    uint32 thre = io_port_in(IO_PORT_SERIAL_COM1 + UART_LSR) & UART_LSR_TX_HOLDING_EMPTY;
    if(thre == 0x20) {
        return 1;
    } else {
        return 0;
    }
}

/* ========================================================================= */

void serial_write_line(char * line) {
    if(!serial_initialised)
        return;

    while(*line != '\0') {
        while(serial_is_transmit_empty() == false);
        io_port_out(IO_PORT_SERIAL_COM1, line[0]);
        line++;
    }
}

/* ========================================================================= */
