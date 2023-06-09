/* ========================================================================= */
/* Virtual Memory Manager                                                    */
/* ========================================================================= */
#pragma once

#include <rotary/options.h>
#include <rotary/types.h>
#include <rotary/logging.h>
#include <rotary/paging.h>
#include <rotary/pmm.h>

#undef  LOG_TAG
#define LOG_TAG "memory"

/* ========================================================================= */

#define HEAP_START_ADDR     0xC0080000
#define HEAP_MAX_SIZE       (65536 * 16)

#define BLOCK_FREE      0
#define BLOCK_USED      1

#define ROUND_UP_4(x) (((x) + 3) & ~3)

#define VMM_SUCCESS             0
#define VMM_ERR_PHYS_EXHAUSTED  -1
#define VMM_ERR_INVALID_ADDR    -2
#define VMM_ERR_INVALID_SIZE    -3

#define KMALLOC_FAIL            -1

/* ========================================================================= */

typedef struct block_header {
    uint32   size;
    uint32    used;
    struct block_header * prev_free_block;
    struct block_header * next_free_block;
} __attribute__((packed)) block_header;

typedef struct {
    uint32   size;
    uint32    used;
} __attribute__((packed)) block_trailer;

/* ========================================================================= */

extern void * heap_start_vaddr;
extern void * heap_end_vaddr;
extern page_directory_entry * vmm_kernel_pd;

/* ========================================================================= */

uint32  vmm_init();
void    vmm_print();

uint32  vmm_alloc_pages(void * virtual_addr, uint32 size);

void    heap_init(void * heap_start_vaddr);
void    heap_update_block(void * addr, uint32 size, uint32 used);

void    heap_insert_free_block(block_header * header);
void    heap_remove_free_block(block_header * header);

void    heap_create_new_block(void * addr, uint32 size, uint32 used, void * prev_free_block, void * next_free_block);
void    heap_coalesce_free_block(block_header * free_block);

void *  kmalloc(uint32 size);
int     kfree(void * ptr);

/* ========================================================================= */
