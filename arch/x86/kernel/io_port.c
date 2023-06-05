#include <rotary/io_port.h>

/* ========================================================================= */

// Read from an I/O port
uint8 io_port_in(uint16 port_num) {
    uint8 value;
    asm("in %%dx, %%al" : "=a"(value) : "d"(port_num));
    return value;
}

/* ========================================================================= */

// Write to an I/O port
void io_port_out(uint16 port_num, uint8 data) {
    asm("out %%al, %%dx" :: "a"(data), "d" (port_num));
}

/* ========================================================================= */
