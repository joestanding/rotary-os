#include <rotary/paging.h>

/* ========================================================================= */

uint32 paging_load_directory(page_directory_entry * page_directory) {
    if(!IS_4K_ALIGNED((uint32)page_directory)) {
        printk(LOG_DEBUG, "paging_load_directory(): PD is not page aligned!\n");
        return -1;
    }
    asm volatile("mov %0, %%cr3" :: "r"((uint32)page_directory) : "memory");
    return 0;
}

/* ========================================================================= */

uint32 paging_init_directory(page_directory_entry * page_directory) {
    if(!IS_4K_ALIGNED((uint32)page_directory)) {
        printk(LOG_DEBUG, "paging_init_directory(): Table address (0x%x) is not page aligned!\n", &page_directory);
        return -1;
    }

    // Set up our 1024 page directory entries
    for(uint32 i = 0; i < 1024; i++) {
        page_directory[i].present  = PAGE_NOT_PRESENT;
        page_directory[i].writable = PAGE_WRITABLE;
        page_directory[i].user     = PAGE_KERNEL_ONLY;
    }
    return 0;
}

/* ========================================================================= */

uint32 paging_init_table(page_table_entry * page_table) {
    if(!IS_4K_ALIGNED((uint32)page_table)) {
        printk(LOG_DEBUG, "paging_init_table(): Table address (0x%x) is not page aligned!\n", &page_table);
        return -1;
    }

    // Set up our 1024 page table entries
    for(uint32 i = 0; i < 1024; i++) {
        page_table[i].present  = PAGE_NOT_PRESENT;
        page_table[i].writable = PAGE_WRITABLE;
        page_table[i].user     = PAGE_KERNEL_ONLY;
    }
    return 0;
}

/* ========================================================================= */

uint32 paging_map(page_directory_entry * page_directory, void * virtual_addr, void * physical_addr, uint32 size) {
    if(!IS_4K_ALIGNED(size)) {
        printk(LOG_DEBUG, "paging_map(): Size (%d) is not 4K aligned!\n", size);
        return -1;
    }

    //TODO: handle different page sizes
    uint32 mapping_count = ALIGN_UP_4K(size) / 4096;
    uint32 working_virt_addr = (uint32)virtual_addr;
    uint32 working_phys_addr = (uint32)physical_addr;

    // Iterate
    for(uint32 i = 0; i < mapping_count; i++) {
        // Calculate the relevant directory and table indices
        uint32 dir_index = PAGE_DIRECTORY_INDEX(working_virt_addr);
        uint32 tbl_index = PAGE_TABLE_INDEX(working_virt_addr);
        
        // Check if a page table is already allocated at this index
        page_table_entry * page_table = NULL;
        if(page_directory[dir_index].present == PAGE_NOT_PRESENT)  {
            // Allocate a physical frame for our new page table and initialise all entries as not present
            page_table = pmm_alloc_frame(SIZE_4K);
            paging_init_table(page_table);
            page_directory[dir_index].address = (uint32)page_table >> 12;
            page_directory[dir_index].present = PAGE_PRESENT;
        } else {
            // A page table is already allocated, use that
            page_table = (page_table_entry*)(page_directory[dir_index].address << 12);
        }

        // Insert our page table entry
        page_table[tbl_index].present  = PAGE_PRESENT;
        page_table[tbl_index].writable = PAGE_WRITABLE;
        page_table[tbl_index].user     = PAGE_KERNEL_ONLY;
        page_table[tbl_index].address  = (uint32)working_phys_addr >> 12;

        // Increment the addresses we're working on
        working_virt_addr += SIZE_4K;
        working_phys_addr += SIZE_4K;
    }
    return NULL;
}

/* ========================================================================= */
