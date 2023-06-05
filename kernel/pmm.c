#include <rotary/pmm.h>

/* ========================================================================= */

extern uint32 * KERNEL_PHYS_START;
extern uint32 * KERNEL_PHYS_END;

uint32 pmm_memory_bitmap[4096];
uint32 pmm_memory_start = 0;
uint32 pmm_memory_end   = 0;
uint32 pmm_frames_max   = 0;
uint32 pmm_frames_used  = 0;

/* ========================================================================= */

uint32 pmm_init() {
    if(pmm_memory_start == 0x00 && pmm_memory_end == 0x00) {
        printk(LOG_DEBUG, "PMM memory ranges have not been configured, halting!\n");
        //TODO: replace with kernel panic
        debug_break();
    }

    uint32 pmm_max_frames = (pmm_memory_end - pmm_memory_start) / PMM_FRAME_SIZE;
    printk(LOG_DEBUG, "Frames available in allocated physical memory region: %d\n", pmm_max_frames);

    return 1;
}

/* ========================================================================= */

void pmm_set_memory_range(uint32 start_addr, uint32 end_addr) {
    pmm_memory_start = start_addr;
    pmm_memory_end   = end_addr;
    pmm_frames_max   = (end_addr - start_addr) / PMM_FRAME_SIZE;
    printk(LOG_DEBUG, "Avail. memory range set to 0x%x -> 0x%x, %d frames\n", start_addr, end_addr, pmm_frames_max);
}

/* ========================================================================= */

void pmm_set_frame_used(uint32 frame_num) {
    if(frame_num >= pmm_frames_max) {
        printk(LOG_DEBUG, "pmm_set_frame_used(): Frame %d is beyond max frame bounds (%d)!\n", frame_num,
                pmm_frames_max);
        return;
    }

    // We could easily write this in one line but I've broken it out for comprehension's sake
    // As we can only index into the bitmap by byte, calculate the byte we need
    uint32 index = frame_num / 32;
    // Create the bit mask for the OR that'll set the one bit we want set
    uint32 bit_mask = (1 << (frame_num % 32));
    pmm_memory_bitmap[index] |= bit_mask;
    pmm_frames_used++;
}

/* ========================================================================= */

void pmm_set_frame_free(uint32 frame_num) {
    if(frame_num >= pmm_frames_max) {
        printk(LOG_DEBUG, "pmm_set_frame_free(): Frame %d is beyond max frame bounds (%d)!\n", frame_num,
                pmm_frames_max);
        return;
    }

    uint32 index = frame_num / 32;
    uint32 bit_mask = (1 << (frame_num % 32));
    pmm_memory_bitmap[index] &= ~ bit_mask;
    pmm_frames_used--;
}

/* ========================================================================= */

uint32 pmm_get_first_free_frame() {
    for(uint32 i = 0; i < pmm_frames_max / 32; i++) {
        for(uint32 bit = 0; bit < 32; bit++) {
            uint32 bit_mask = (uint32)1 << bit;
            if(!(pmm_memory_bitmap[i] & bit_mask)) {
                //TODO: explain arithmetic
                return i * 4 * 8 + bit;
            }
        }
    }
    return PMM_NO_FRAMES_AVAIL;
}

/* ========================================================================= */

void * pmm_alloc_frame() {
    uint32 first_free_frame = pmm_get_first_free_frame();
    pmm_set_frame_used(first_free_frame);
    void * frame_phys_addr = (void*)pmm_memory_start + (first_free_frame * PMM_FRAME_SIZE);
    //printk(LOG_DEBUG, "pmm_alloc_frame(): Allocating frame num %d at physaddr 0x%x\n", first_free_frame, frame_phys_addr);
    return frame_phys_addr;
}

/* ========================================================================= */

void pmm_dealloc_frame(void * frame_phys_addr) {
    uint32 frame_num = (uint32)(((uint32)frame_phys_addr - pmm_memory_start) / PMM_FRAME_SIZE);
    //printk(LOG_DEBUG, "pmm_alloc_frame(): Deallocating frame num %d at physaddr 0x%x\n", frame_num, frame_phys_addr);
    pmm_set_frame_free(frame_num);
}

/* ========================================================================= */

uint32 pmm_align_4k(uint32 address) {
    const uint32 alignment = 4096;
    return (address + alignment - 1) & ~(alignment - 1);
}

/* ========================================================================= */

void pmm_print() {
    printk(LOG_DEBUG, "Physical Memory Manager\n");
    printk(LOG_DEBUG, "-----------------------\n");
    printk(LOG_DEBUG, "Frame Size:      %d bytes\n", PMM_FRAME_SIZE);
    printk(LOG_DEBUG, "Max Frames:      %d\n", pmm_frames_max);
    printk(LOG_DEBUG, "Frames Used:     %d\n", pmm_frames_used);
    printk(LOG_DEBUG, "Kernel Start:    0x%x\n", &KERNEL_PHYS_START);
    printk(LOG_DEBUG, "Kernel End:      0x%x\n", &KERNEL_PHYS_END);
    printk(LOG_DEBUG, "Mem. Used:       %d bytes\n", pmm_frames_used * PMM_FRAME_SIZE);
    printk(LOG_DEBUG, "Mem. Start:      0x%x\n", pmm_memory_start);
    printk(LOG_DEBUG, "Mem. End:        0x%x\n", pmm_memory_end);
    printk(LOG_DEBUG, "Bitmap Addr:     0x%x\n", &pmm_memory_bitmap);
}

/* ========================================================================= */
