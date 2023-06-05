#include <rotary/pic8259.h>

// Send an End-of-Interrupt signal to the PIC
uint32 pic_send_eoi(uint8 irq) {
    if(irq >= 8)
        io_port_out(IO_PORT_PIC2_CMD, IO_CMD_PIC_EOI);
    io_port_out(IO_PORT_PIC1_CMD, IO_CMD_PIC_EOI);

    return 1;
}

// Configure the PIC and offset any IRQs so they don't conflict with the CPU
uint32 pic_init(uint8 master_offset, uint8 slave_offset) {
    // Begin PIC initialisation - set bit 4 to indicate this is ICW1, set bit 0 to
    // indicate an ICW4 will be sent later
    printk(LOG_DEBUG, "Sending ICW1_INIT to PIC1\n");
    io_port_out(IO_PORT_PIC1_CMD, IO_CMD_PIC_ICW1_INIT | IO_CMD_PIC_ICW1_ICW4);
    printk(LOG_DEBUG, "Sending ICW1_INIT to PIC2\n");
    io_port_out(IO_PORT_PIC2_CMD, IO_CMD_PIC_ICW1_INIT | IO_CMD_PIC_ICW1_ICW4);

    // Set master and slave PIC vector offset
    // We offset the interrupts so that they don't conflict with Intel's reserved
    // interrupts up to 0x1F
    printk(LOG_DEBUG, "Setting PIC offsets [PIC1: %d | PIC2: %d]\n", master_offset, slave_offset);
    io_port_out(IO_PORT_PIC1_DATA, master_offset);
    io_port_out(IO_PORT_PIC2_DATA, slave_offset);

    // Configure slave PIC at IRQ2
    printk(LOG_DEBUG, "Configuring slave PIC at IRQ2\n");
    io_port_out(IO_PORT_PIC1_DATA, IO_CMD_PIC_ICW3_MASTER);

    // Configure slave PIC as secondary
    printk(LOG_DEBUG, "Configuring slave PIC as secondary\n");
    io_port_out(IO_PORT_PIC2_DATA, IO_CMD_PIC_ICW3_SLAVE);

    // 8086 mode 
    printk(LOG_DEBUG, "Enabling 8086 mode on PICs\n");
    io_port_out(IO_PORT_PIC1_DATA, IO_CMD_PIC_ICW4_8086_MODE);
    io_port_out(IO_PORT_PIC2_DATA, IO_CMD_PIC_ICW4_8086_MODE);

    // Enable all IRQs
    printk(LOG_DEBUG, "Enabling all IRQs\n");
    io_port_out(IO_PORT_PIC1_DATA, 0x00);
    io_port_out(IO_PORT_PIC2_DATA, 0x00);

    return 1;
}
