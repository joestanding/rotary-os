/* ========================================================================= */
/* Virtual Memory Manager                                                    */
/* ========================================================================= */
#pragma once

#include <rotary/options.h>
#include <rotary/types.h>
#include <rotary/logging.h>
#include <rotary/paging.h>
#include <rotary/pmm.h>
#include <rotary/kheap.h>
#include <rotary/cpuid.h>

#undef  LOG_TAG
#define LOG_TAG "memory"

/* ========================================================================= */

#define ROUND_UP_4(x) (((x) + 3) & ~3)

#define VMM_SUCCESS             0
#define VMM_ERR_PHYS_EXHAUSTED  -1
#define VMM_ERR_INVALID_ADDR    -2
#define VMM_ERR_INVALID_SIZE    -3

/* ========================================================================= */

extern void * heap_start_vaddr;
extern void * heap_end_vaddr;
extern page_directory_entry * vmm_kernel_pd;

/* ========================================================================= */

uint32  vmm_init();
void    vmm_print();
uint32  vmm_alloc_pages(void * virtual_addr, uint32 size);
uint32  vmm_map_kernel(page_directory_entry * pd);

/* ========================================================================= */
