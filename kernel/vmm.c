#include <rotary/vmm.h>

extern uint32 KERNEL_VIRT_END;

page_directory_entry * vmm_current_pd = NULL;
page_directory_entry * vmm_kernel_pd = NULL;
void * heap_start_vaddr = NULL;
void * heap_end_vaddr = NULL;

/* ========================================================================= */
/* Virtual Memory Management                                                 */
/* ========================================================================= */

uint32 vmm_init() {
    // Allocate a physical page for our default page directory
    vmm_kernel_pd = (page_directory_entry*)PHY_TO_VIR(pmm_alloc_frame());
    vmm_current_pd = vmm_kernel_pd;
    memset(vmm_current_pd, 0, PAGE_SIZE);

    if(!vmm_current_pd) {
        printk(LOG_DEBUG, "vmm_init(): Failed to allocate initial page directory frame\n");
        return -1;
    }
    printk(LOG_DEBUG, "vmm_init(): Allocated initial page dir. frame at 0x%x\n", vmm_current_pd);

    // Initialise the directory and set up the kernel mapping
    paging_init_directory(vmm_current_pd);
    vmm_map_kernel(vmm_current_pd);
    paging_load_directory((page_directory_entry*)VIR_TO_PHY(vmm_kernel_pd));

    // Load the paging directory into CR3 and begin using it
    printk(LOG_DEBUG, "vmm_init(): Loading newly created page directory..\n");
    
    // Locate where we're going to start our kernel heap, and allocate memory for it
    printk(LOG_DEBUG, "vmm_init(): Allocating initial memory for the heap allocator..\n");
    uint32 heap_size = HEAP_MAX_SIZE + sizeof(block_header) + sizeof(block_trailer);
    uint32 page_count = ALIGN_UP_4K(heap_size) / PAGE_SIZE;

    heap_start_vaddr = (void*)ALIGN_UP_4K((uint32)&KERNEL_VIRT_END + PMM_FRAME_SIZE);
    heap_end_vaddr   = (void*)(heap_start_vaddr + HEAP_MAX_SIZE + sizeof(block_header) + sizeof(block_trailer));

    for(uint32 page_index = 0; page_index < page_count; page_index++) {
        void * phys_page_addr = pmm_alloc_frame();
        if(!phys_page_addr) {
            // TODO: Unmap any pages we mapped this cycle
            return VMM_ERR_PHYS_EXHAUSTED;
        }
    }
    //vmm_alloc_pages(heap_start_vaddr, HEAP_MAX_SIZE + sizeof(block_header) + sizeof(block_trailer));
    heap_init(heap_start_vaddr);
    return 1;
}

/* ========================================================================= */

uint32 vmm_map(void * virtual_addr, void * physical_addr, uint32 size) {
    paging_map(vmm_current_pd, virtual_addr, physical_addr, size);
    return 0;
}

/* ========================================================================= */

uint32 vmm_alloc_pages(void * virtual_addr, uint32 size) {
    // TODO: Consider logic to use larger (4MB) pages if possible
    
    // TODO: Validate address
    
    // TODO: Validate size
    if(size <= 0) {
        return VMM_ERR_INVALID_SIZE;
    }

    // Figure out how many pages we'll need to allocate to satisfy this request
    uint32 page_count = ALIGN_UP_4K(size) / PAGE_SIZE;
    printk(LOG_DEBUG, "vmm_alloc_pages(): Req. %d bytes, we need %d pages\n", size, page_count);

    void * current_vaddr = virtual_addr;
    for(uint32 page_index = 0; page_index < page_count; page_index++) {
        void * phys_page_addr = pmm_alloc_frame();
        if(!phys_page_addr) {
            // TODO: Unmap any pages we mapped this cycle
            return VMM_ERR_PHYS_EXHAUSTED;
        }
        paging_map(vmm_current_pd, current_vaddr, phys_page_addr, SIZE_4K);
        current_vaddr = (void*)((char*)current_vaddr + PAGE_SIZE);
    }

    // TODO: Review how to remove this to prevent TLB flushes
    paging_load_directory((page_directory_entry*)VIR_TO_PHY(vmm_current_pd));
    return 0;
}

/* ========================================================================= */

uint32 vmm_map_kernel(page_directory_entry * pd) {
    uint32 current_addr = 0;
    if(cpuid_check_pse()) {
        // Use 4MB pages for efficiency
        for(uint32 i = 768; i < 1024; i++) {
            pd[i].present   = PAGE_PRESENT;
            pd[i].writable  = PAGE_WRITABLE;
            pd[i].user      = PAGE_KERNEL_ONLY;
            pd[i].page_size = PAGE_SIZE_4M;
            pd[i].address   = current_addr >> 12;
            current_addr += SIZE_4M;
        }
    } else {
        // TODO: Use 4KB pages
    }

    return 1;
}

/* ========================================================================= */

void vmm_print() {
    printk(LOG_INFO, "Virtual Memory Manager\n");
    printk(LOG_INFO, "----------------------\n");
    printk(LOG_INFO, "Kernel PD:     0x%x\n", &vmm_kernel_pd);
}

/* ========================================================================= */
