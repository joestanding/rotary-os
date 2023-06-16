#include <rotary/kheap.h>

/* ========================================================================= */

void heap_init(void * heap_start_vaddr) {
    printk(LOG_DEBUG, "heap_init(): Initialising heap at virt. addr 0x%x\n", heap_start_vaddr);
    printk(LOG_DEBUG, "heap_init(): Creating first heap block at 0x%x of size %d bytes\n", heap_start_vaddr, HEAP_MAX_SIZE);

    // Zero out the memory we're going to use
    memset(heap_start_vaddr, 0, HEAP_MAX_SIZE + sizeof(block_header) + sizeof(block_trailer));

    // Create a new block header at the start of our heap memory space
    uint32 * new_block_addr = (uint32*)heap_start_vaddr;
    block_header * blk_header = (block_header*)new_block_addr;

    // Set the attributes of the first block header
    blk_header->size = HEAP_MAX_SIZE;
    blk_header->used = 0;
    blk_header->prev_free_block = (block_header*)0x00000000;
    blk_header->next_free_block = (block_header*)0x00000000;

    // Set the attributes of the first block trailer
    block_trailer * blk_trailer = (block_trailer*)((char*)new_block_addr + sizeof(block_header) + 
            blk_header->size);
    blk_trailer->size = HEAP_MAX_SIZE;
    blk_trailer->used = 0;

    printk(LOG_DEBUG, "heap_init(): Initialised heap\n");
}

/* ========================================================================= */

// Update the size and used attributes of a given heap block allocation
void heap_update_block(void * addr, uint32 size, uint32 used) {
    block_header *  header  = (block_header*)addr;
    header->size = size;
    header->used = used;

    block_trailer * trailer = (block_trailer*)((char*)header + sizeof(block_header) +
            header->size);
    trailer->size = size;
    trailer->used = used;

    printk(LOG_DEBUG, "heap_update_block() - updated addr 0x%x sz %d\n", addr, size);
    printk(LOG_DEBUG, "heap_update_block() - new trailer at addr 0x%x\n", trailer);
}

/* ========================================================================= */

void heap_insert_free_block(block_header * header) {
    printk(LOG_DEBUG, "Inserting free block at addr 0x%x\n", &header);
    // TODO: Implement 
}

/* ========================================================================= */

void heap_remove_free_block(block_header * header) {
    printk(LOG_DEBUG, "Removing free block at addr 0x%x\n", &header);
    if(header->prev_free_block != 0)
        ((block_header*)header->prev_free_block)->next_free_block = header->next_free_block;

    if(header->next_free_block != 0)
        ((block_header*)header->next_free_block)->prev_free_block = header->prev_free_block;
}

/* ========================================================================= */

void heap_create_new_block(void * addr, uint32 size, uint32 used, void * prev_free_block, void * next_free_block) {
    printk(LOG_DEBUG, "Creating new block at addr 0x%x of size %d bytes, prev. blk 0x%x, next blk 0x%x\n", addr,
                                                                                                           size,
                                                                                                           prev_free_block,
                                                                                                           next_free_block);

    block_header * header   = (block_header*)addr;
    header->size = size;
    header->used = used;
    header->prev_free_block = prev_free_block;
    header->next_free_block = next_free_block;

    block_trailer * trailer = (block_trailer*)((char*)header + sizeof(block_header) +
            header->size);
    trailer->size = size;
    trailer->used = used;
}

/* ========================================================================= */

void heap_coalesce_free_block(block_header * free_block) {
    block_trailer * prev_trailer = (block_trailer*)free_block - 1;
    block_header  * prev_header  = (block_header*)((char*)prev_trailer - prev_trailer->size -
            sizeof(block_header));

    block_header * next_header = (block_header*)((char*)free_block + sizeof(block_header) +
            free_block->size + sizeof(block_trailer));
    block_trailer * next_trailer = (block_trailer*)((char*)next_header + sizeof(block_header) +
            next_header->size);

    if(next_header->size != 0 && next_header->used == BLOCK_FREE) {
        uint32 new_size = free_block->size +
            sizeof(block_trailer) +
            sizeof(block_header) +
            next_header->size;

        free_block->next_free_block = next_header->next_free_block;
        free_block->size = new_size;
        next_trailer->size = new_size;
        next_trailer->used = BLOCK_FREE;
    }

    if(prev_header->size != 0 && prev_header->used == BLOCK_FREE) {
        uint32 new_size = prev_header->size +
            sizeof(block_trailer) +
            sizeof(block_header) +
            free_block->size;

        prev_header->size = new_size;
        prev_header->next_free_block = free_block->next_free_block;
        next_trailer->size = new_size;
        next_trailer->used = BLOCK_FREE;
    }
}

/* ========================================================================= */

void * kmalloc(uint32 size) {
    if(size <= 0) {
        return (void*)0;
    }

    printk(LOG_TRACE, "kmalloc(): Asked to allocate %d bytes\n", size);

    // Round up to the closest multiple of 4
    size = ROUND_UP_4(size); 

    // Identifying the best free block
    printk(LOG_TRACE, "kmalloc(): Starting with block at 0x%x\n", heap_start_vaddr);
    block_header * current_block   = (block_header*)heap_start_vaddr;
    block_header * best_block      = NULL;
    while(current_block) {
        if(current_block->used == 0) {
            if(best_block == NULL) {
                if(current_block->size >= size) {
                    best_block = current_block;
                }
            } else {
                if(current_block->size >= size && current_block->size < best_block->size) {
                    best_block = current_block;
                }
            }
        }
        printk(LOG_TRACE, "kmalloc(): Updating current_block in search to 0x%x\n", current_block->next_free_block);
        current_block = (block_header*)current_block->next_free_block;
    }

    // If we couldn't find an appropriate block, error out
    if(best_block == NULL) {
        printk(LOG_TRACE, "kmalloc(): No suitable block found!\n");
        return (void*)0;
    }

    // Case 1: Perfect Fit
    // The free block we found is the exact right size. We can just update the header and trailer.
    if(best_block->size == size) {
        printk(LOG_TRACE, "kmalloc(): Perfect fit, best_block->size (%d)\n", best_block->size);

        // Update the header of the block to our new values
        heap_update_block((void*)best_block, size, BLOCK_USED);

        // Update the linked list to no longer include this previously free block
        heap_remove_free_block(best_block);

        return (void*)best_block + sizeof(block_header);
    }

    // Case 2: Big Block Splitting
    // The free block we found is too big, we'll split it into two.
    if(best_block->size > size) {
        printk(LOG_DEBUG, "kmalloc(): BB split, addr: 0x%x, bb->size: %d, req. sz: %d\n", best_block, best_block->size,
                size);

        // Calculate the starting address for our new block within the existing
        void * new_block_addr = (void*)best_block + 
            sizeof(block_header) + best_block->size - sizeof(block_header) - size;

        // Re-calculate the original block's size
        uint32 old_block_size = best_block->size - sizeof(block_header) - size - sizeof(block_trailer);

        // Update the block we're carving with its new size and create a new trailer
        heap_update_block((void*)best_block, old_block_size, BLOCK_FREE);

        // Insert our new block
        heap_create_new_block(new_block_addr, size, BLOCK_USED, (void*)best_block, (void*)best_block->next_free_block);

        return (void*)new_block_addr + sizeof(block_header);
    }

    return (void*)0;
}

/* ========================================================================= */

int kfree(void * ptr) {
    printk(LOG_DEBUG, "kfree(): Freeing 0x%x\n", ptr);

    if(ptr == NULL) {
        printk(LOG_DEBUG, "kfree(): Invalid ptr of NULL received!\n");
        return 0;
    }

    if(ptr < heap_start_vaddr || ptr > heap_end_vaddr) {
        printk(LOG_DEBUG, "kfree(): Ptr (0x%x) is out of heap bounds (0x%x -> 0x%x)\n",
                ptr,
                heap_start_vaddr,
                heap_end_vaddr);
        return 0;
    }

    block_header * to_free_block = (block_header*)ptr - 1;
    block_trailer * to_free_trailer = (block_trailer*)((char*)to_free_block + sizeof(block_header) + 
            to_free_block->size);

    // If enabled, zero out the memory upon freeing
    if(KERNEL_MEMZERO_ON_FREE)
        memset(ptr, 0, to_free_block->size);

    // Set the block as no longer used
    to_free_block->used = 0;
    to_free_trailer->used = 0;

    // Update the free list to now include this freed block
    // 1. Set the newly freed block's next_free_block to the previous free block's next_free_block
    //    This works because it will point to whatever free block is after the block we've just freed.
    // 2. Set the previous free block's next_free_block to the newly freed block
    block_header * current_block = (block_header*)heap_start_vaddr;
    block_header * last_free_block = current_block;

    while(current_block->size != 0) {
        if(current_block == to_free_block) {
            // Patch up the free list to insert our newly freed block
            current_block->next_free_block = last_free_block->next_free_block;
            current_block->prev_free_block = last_free_block;
            last_free_block->next_free_block = current_block;

            // If we're adjacent to some free blocks, coalesce them into one single block
            heap_coalesce_free_block(current_block);
            break;
        }

        // Keep track of the last free block that we've seen
        if(current_block->used == 0 && current_block != to_free_block)
            last_free_block = current_block;

        // Calculate the next block in the list
        current_block = (block_header*)((char*)current_block + sizeof(block_header) + current_block->size + sizeof(block_trailer));
    }

    return 1;
}

/* ========================================================================= */
