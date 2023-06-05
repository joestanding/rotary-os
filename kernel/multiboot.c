#include <rotary/multiboot.h>

extern uint32 KERNEL_PHYS_END;

void udivmod64(uint64 dividend, uint64 divisor, uint64 *quotient, uint64 *remainder) {
    uint64 q = 0;
    uint64 r = 0;
    int i;

    for (i = 63; i >= 0; i--) {
        r <<= 1;
        r |= (dividend >> i) & 1;

        if (r >= divisor) {
            r -= divisor;
            q |= (uint64)1 << i;
        }
    }

    if (quotient) {
        *quotient = q;
    }

    if (remainder) {
        *remainder = r;
    }
}

void uint64_to_str(uint64 value, char *str, int base) {
    char *ptr = str, *ptr1 = str, tmp_char;
    uint64 quotient, remainder;

    // Calculate the string representation of the number in the given base
    do {
        udivmod64(value, base, &quotient, &remainder);
        value = quotient;
        *ptr++ = "0123456789abcdef"[remainder];
    } while (value);

    // Null-terminate the string
    *ptr-- = '\0';

    // Reverse the string
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}


uint32 multiboot_parse(uint32 mboot_magic, multiboot_info_t * info) {

    if(mboot_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        printk(LOG_DEBUG, "Invalid Multiboot magic bytes! Got 0x%x, expected 0x%x\n", mboot_magic, 
                MULTIBOOT_BOOTLOADER_MAGIC);
        return 0;
    }

    printk(LOG_DEBUG, "--- Multiboot Info ---\n");
    printk(LOG_DEBUG, "Flags:    0x%x\n", info->flags);
    printk(LOG_DEBUG, "MemLow:   0x%x\n", info->mem_lower);
    printk(LOG_DEBUG, "MemUpp:   0x%x\n", info->mem_upper);
    printk(LOG_DEBUG, "MmapLen:  %d\n", info->mmap_length);
    printk(LOG_DEBUG, "MmapAddr: 0x%x\n", info->mmap_addr);

    if (info->flags & MULTIBOOT_INFO_MEMORY) {
        printk(LOG_DEBUG, "Lower memory: %dKB\n", info->mem_lower);
        printk(LOG_DEBUG, "Upper memory: %dKB\n", info->mem_upper);
    }

    // Print boot device information
    if (info->flags & MULTIBOOT_INFO_BOOTDEV) {
        printk(LOG_DEBUG, "Boot device: 0x%x\n", info->boot_device);
    }

    // Print command line information
    if (info->flags & MULTIBOOT_INFO_CMDLINE) {
        printk(LOG_DEBUG, "Command line (at 0x%x): %s\n", (char *)info->cmdline, (char *)info->cmdline);
    }

    // Print modules information
    if (info->flags & MULTIBOOT_INFO_MODS) {
        printk(LOG_DEBUG, "Modules count: %d\n", info->mods_count);

        multiboot_module_t *mod;
        for (uint32 i = 0; i < info->mods_count; i++) {
            mod = (multiboot_module_t *)(info->mods_addr + i * sizeof(multiboot_module_t));
            printk(LOG_DEBUG, "Module %d start: 0x%x\n", i, mod->mod_start);
            printk(LOG_DEBUG, "Module %d end: 0x%x\n", i, mod->mod_end);
            printk(LOG_DEBUG, "Module %d command line: %s\n", i, (char *)mod->cmdline);
        }
    }

    // Print drive information
    /*
    if(info->flags & MULTIBOOT_INFO_DRIVE_INFO) {
        multiboot_drive_info_t * drive_info;
        printk(LOG_DEBUG, "Drive count: %d\n", info->drives_length);
        for(uint32 i = 0; i < info->drives_length; i++) {
            drive_info = (multiboot_drive_info_t*)(info->drives_addr + i);
            printk("Drive num: %d\n", (uint32)drive_info->drive_number);
        }
    }
    */

    // Print memory map information
    if (info->flags & MULTIBOOT_INFO_MEM_MAP) {
        multiboot_memory_map_t *mmap;

        for (mmap = (multiboot_memory_map_t *)info->mmap_addr;
             (unsigned long)mmap < info->mmap_addr + info->mmap_length;
             mmap = (multiboot_memory_map_t *)((unsigned long)mmap + mmap->size + sizeof(mmap->size))) {

            const char* type = (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) ? "Available" : "Reserved";
            
            char start_addr[32];
            char end_addr[32];
            uint64_to_str(mmap->addr, start_addr, 16);
            uint64_to_str(mmap->addr + mmap->len, end_addr, 16);


            printk(LOG_DEBUG, "Memory region: 0x%s -> 0x%s (%s)\n",
                   start_addr,
                   end_addr,
                   type);

            if(mmap->addr <= (uint32)&KERNEL_PHYS_END && (mmap->addr + mmap->len) >= (uint32)&KERNEL_PHYS_END && mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
                uint32 aligned_end_addr = pmm_align_4k((uint32)&KERNEL_PHYS_END);
                printk(LOG_DEBUG, "Found appropriate mem. region, start: 0x%x, end: 0x%x\n", mmap->addr, (mmap->addr + mmap->len));
                printk(LOG_DEBUG, "Setting page frame range to 0x%x -> 0x%x\n", aligned_end_addr, (mmap->addr + mmap->len));
                pmm_set_memory_range(aligned_end_addr, mmap->addr + mmap->len);
            }
        }
    }

    // Print boot loader name
    if (info->flags & MULTIBOOT_INFO_BOOT_LOADER_NAME) {
        printk(LOG_DEBUG, "Boot loader name: %s\n", (char *)info->boot_loader_name);
    }

    return 1;
}
