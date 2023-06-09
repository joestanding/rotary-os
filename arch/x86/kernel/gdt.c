#include <rotary/gdt.h>

extern void gdt_flush();

/* ========================================================================= */

gdt_descriptor descriptor;
gdt_entry null_entry;
gdt_entry kernel_code_entry;
gdt_entry kernel_data_entry;
gdt_entry user_code_entry;
gdt_entry user_data_entry;
gdt_entry tss_entry;
tss tss_obj;

/* ========================================================================= */

uint32 gdt_init() {
    uint16 gdt_size = (sizeof(gdt_entry) * 6) - 1;

    printk(LOG_DEBUG, "Installing GDT of size %d at 0x%x\n", gdt_size, &null_entry);
    printk(LOG_DEBUG, "GDT descriptor at 0x%x\n", &descriptor);

    descriptor.address = (uint32)&null_entry;
    descriptor.size = gdt_size;

    // Kernel code
    gdt_encode_entry(&kernel_code_entry, 0x00000000, 0xFFFFFFFF, 0xCF, GDT_ACCESS_TYPE_SEGMENT |
                                                                       GDT_ACCESS_PRESENT |
                                                                       GDT_ACCESS_PRIV_KERNEL |
                                                                       GDT_ACCESS_EXECUTABLE |
                                                                       GDT_ACCESS_NONCONFORMING |
                                                                       GDT_ACCESS_READABLE);
    
    // Kernel data
    gdt_encode_entry(&kernel_data_entry, 0x00000000, 0xFFFFFFFF, 0xCF, GDT_ACCESS_TYPE_SEGMENT |
                                                                       GDT_ACCESS_PRESENT |
                                                                       GDT_ACCESS_PRIV_KERNEL |
                                                                       GDT_ACCESS_DATA |
                                                                       GDT_ACCESS_DIRECTION_UP |
                                                                       GDT_ACCESS_WRITABLE);

    // User code
    gdt_encode_entry(&user_code_entry, 0x00000000, 0xFFFFFFFF, 0xCF, GDT_ACCESS_TYPE_SEGMENT |
                                                                     GDT_ACCESS_PRESENT |
                                                                     GDT_ACCESS_PRIV_USER |
                                                                     GDT_ACCESS_EXECUTABLE |
                                                                     GDT_ACCESS_NONCONFORMING |
                                                                     GDT_ACCESS_READABLE);

    // User data
    gdt_encode_entry(&user_data_entry, 0x00000000, 0xFFFFFFFF, 0xCF, GDT_ACCESS_TYPE_SEGMENT |
                                                                     GDT_ACCESS_PRESENT |
                                                                     GDT_ACCESS_PRIV_USER |
                                                                     GDT_ACCESS_DATA |
                                                                     GDT_ACCESS_DIRECTION_UP |
                                                                     GDT_ACCESS_WRITABLE);

    // Task State Segment
    tss * task_seg = tss_get();
    gdt_encode_entry(&tss_entry, (uint32)task_seg, sizeof(tss), 0x40, 0x89);

    // Load the GDT descriptor
    asm volatile("lgdt %0" :: "m"(descriptor));

    printk(LOG_DEBUG, "GDT loaded!\n");

    // Update the segment registers, as they won't be correct after GRUB set them
    gdt_flush();

    // Load the task register
    tss_flush();

    return 1;
}

/* ========================================================================= */

void gdt_encode_entry(gdt_entry * entry, uint32 base, uint32 limit, uint8 flags, uint8 access) {

    printk(LOG_DEBUG, "GDT entry -> addr: 0x%x | base: 0x%x | limit: 0x%x\n", entry, base, limit);

    entry->base_low    = (base & 0xFFFF);
    // We shift the bits we want (17-24 of base) to the right by 16 so that they now
    // occupy the least significant bits (0-7). That allows us to then extract them
    // with the bitwise AND.
    entry->base_middle = (base >> 16) & 0xFF;
    entry->base_high   = (base >> 24) & 0xFF;
    
    entry->limit_low   = (limit & 0xFFFF);
    entry->granularity = (limit >> 16) & 0x0F;

    entry->granularity |= flags & 0xF0;
    entry->access_flags = access;
}

/* ========================================================================= */
