/* ========================================================================= */
/* Global Descriptor Table                                                   */
/* ========================================================================= */
#pragma once

#include <rotary/types.h>
#include <rotary/tss.h>
#include <rotary/logging.h>

/* ========================================================================= */

#undef  LOG_TAG
#define LOG_TAG "gdt"

// Descriptor Types
#define GDT_ACCESS_TYPE_SEGMENT         0x10
#define GDT_ACCESS_TYPE_SYS_SEGMENT     0x00

// Segment Descriptor
#define GDT_ACCESS_PRESENT           0x80
#define GDT_ACCESS_NOT_PRESENT       0x00
#define GDT_ACCESS_PRIV_KERNEL       0x00
#define GDT_ACCESS_PRIV_USER         0x60
#define GDT_ACCESS_EXECUTABLE        0x08
#define GDT_ACCESS_DATA              0x00
#define GDT_ACCESS_DIRECTION_UP      0x00
#define GDT_ACCESS_DIRECTION_DOWN    0x04
#define GDT_ACCESS_CONFORMING        0x04
#define GDT_ACCESS_NONCONFORMING     0x00
#define GDT_ACCESS_READABLE          0x02
#define GDT_ACCESS_NOT_READABLE      0x00
#define GDT_ACCESS_WRITABLE          0x02
#define GDT_ACCESS_NOT_WRITABLE      0x00

// System Segment Descriptor
#define GDT_ACCESS_16BIT_TSS_AVAIL   0x01
#define GDT_ACCESS_LDT               0x02
#define GDT_ACCESS_16BIT_TSS_BUSY    0x03
#define GDT_ACCESS_32BIT_TSS_AVAIL   0x09
#define GDT_ACCESS_32BIT_TSS_BUSY    0x0B
#define GDT_ACCESS_64BIT_TSS_AVAIL   0x09
#define GDT_ACCESS_64BIT_TSS_BUSY    0x0B

/* ========================================================================= */

// The GDT descriptor points to a list of GDT entries in memory
// The total size of the entries minus one is in `size`, and
// the address of the list is kept in `address`.
typedef struct {
    unsigned short size;
    unsigned int   address;
} __attribute__((packed)) gdt_descriptor;

typedef struct {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char  base_middle;
    unsigned char  access_flags;
    unsigned char  granularity;
    unsigned char  base_high;
} __attribute__((packed)) gdt_entry;

// ========================================================================= //

uint32 gdt_init();
void gdt_encode_entry(gdt_entry * entry, uint32 base, uint32 limit, uint8 flags, uint8 access);

// ========================================================================= //
