#include <rotary/timer.h>

uint64 ticks = 0;

/* ========================================================================= */

uint32 timer_init() {
    printk(LOG_DEBUG, "Configuring Programmable Interrupt Timer interval to 1Hz\n");
    uint32 divisor = 1193180 / 1;

    io_port_out(IO_PORT_PIT_CMD, 0x36);

    uint8 l = (uint8) (divisor & 0xFF);
    uint8 h = (uint8) (divisor >> 8 & 0xFF);

    io_port_out(IO_PORT_PIT_CHAN_0, l);
    io_port_out(IO_PORT_PIT_CHAN_0, h);

    //printk(LOG_DEBUG, "Registering handler for Programmable Interrupt Timer at 0x%x", &timer_tick);
    printk(LOG_DEBUG, "Registering handler for Programmable Interrupt Timer at 0x%x\n", timer_tick);
    register_interrupt_handler(INT_PIT, &timer_tick);

    return 1;
}

/* ========================================================================= */

uint32 timer_tick() {
    ticks++;
    task_schedule();

    return 1;
}

/* ========================================================================= */
