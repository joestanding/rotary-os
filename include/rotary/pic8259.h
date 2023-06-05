/* ========================================================================= */
/* Intel 8259 Programmable Interrupt Controller                              */
/* ========================================================================= */
#pragma once

#include <rotary/types.h>
#include <rotary/io_port.h>
#include <rotary/debug.h>
#include <rotary/logging.h>

/* ========================================================================= */

#undef  LOG_TAG
#define LOG_TAG "pic8259"

#define PIC_MASTER_OFFSET   32
#define PIC_SLAVE_OFFSET    40

// PIC Commands
#define IO_CMD_PIC_EOI  0x20
#define IO_CMD_PIC_ICW1_ICW4        0x01
#define IO_CMD_PIC_ICW1_SINGLE      0x02
#define IO_CMD_PIC_ICW1_INTERVAL4   0x04
#define IO_CMD_PIC_ICW1_LEVEL       0x08
#define IO_CMD_PIC_ICW1_INIT        0x10

#define IO_CMD_PIC_ICW3_MASTER      0x04
#define IO_CMD_PIC_ICW3_SLAVE       0x02

#define IO_CMD_PIC_ICW4_8086_MODE   0x01
#define IO_CMD_PIC_ICW4_AUTO_EOI    0x02
#define IO_CMD_PIC_ICW4_BUF_SLAVE   0x08
#define IO_CMD_PIC_ICW4_BUF_MASTER  0x0C
#define IO_CMD_PIC_ICW4_SFNM        0x10

/* ========================================================================= */

uint32 pic_send_eoi(uint8 irq);
uint32 pic_init(uint8 master_offset, uint8 slave_offset);

/* ========================================================================= */
