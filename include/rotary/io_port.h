/* ========================================================================= */
/* I/O Ports                                                                 */
/* ========================================================================= */
#pragma once

#include <rotary/types.h>

/* ========================================================================= */

// 8259 Programmable Interrupt Controller
#define IO_PORT_PIC1_CMD        0x0020
#define IO_PORT_PIC1_DATA       0x0021
#define IO_PORT_PIC2_CMD        0x00A0
#define IO_PORT_PIC2_DATA       0x00A1


// Programmable Interrupt Timer
#define IO_PORT_PIT_CHAN_0      0x40
#define IO_PORT_PIT_CHAN_1      0x41
#define IO_PORT_PIT_CHAN_2      0x42
#define IO_PORT_PIT_CMD         0x43

// Serial Ports
#define IO_PORT_SERIAL_COM1     0x3F8
#define IO_PORT_SERIAL_COM2     0x2F8
#define IO_PORT_SERIAL_COM3     0x3E8
#define IO_PORT_SERIAL_COM4     0x2E8
#define IO_PORT_SERIAL_COM5     0x5F8
#define IO_PORT_SERIAL_COM6     0x4F8
#define IO_PORT_SERIAL_COM7     0x5E8
#define IO_PORT_SERIAL_COM8     0x4E8

// Keyboard
#define IO_PORT_PS2_KEYBOARD    0x0060

/* ========================================================================= */

void  io_port_out(uint16 port_num, uint8 data);
uint8 io_port_in(uint16 port_num);

/* ========================================================================= */
