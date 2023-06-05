/* ========================================================================= */
/* Physical Memory Manager                                                   */
/* ========================================================================= */
#pragma once

#include <rotary/types.h>
#include <rotary/logging.h>
#include <rotary/debug.h>

/* ========================================================================= */

#undef  LOG_TAG
#define LOG_TAG "pmm"

#define PMM_FRAME_SIZE      4096
#define PMM_MAX_FRAMES      1024 * 512 * PMM_FRAME_SIZE
#define PMM_NO_FRAMES_AVAIL -1

#define PMM_MEMTYPE_RESERVED    0
#define PMM_MEMTYPE_AVAILABLE   1

/* ========================================================================= */

uint32 pmm_init();
void   pmm_print();
void   pmm_set_memory_range(uint32 start_addr, uint32 end_addr);
void   pmm_set_frame_used(uint32 frame_num);
void   pmm_set_frame_free(uint32 frame_num);
uint32 pmm_get_first_free_frame();
void * pmm_alloc_frame();
void * pmm_alloc_frames(uint32 number);
void   pmm_dealloc_frame(void * frame_phys_addr);
uint32 pmm_align_4k(uint32 address);

/* ========================================================================= */
