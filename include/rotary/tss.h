/* ========================================================================= */
/* Task State Segment                                                        */
/* ========================================================================= */
#pragma once

#include <rotary/types.h>
#include <rotary/logging.h>

/* ========================================================================= */

#undef  LOG_TAG
#define LOG_TAG "tss"

/* ========================================================================= */

typedef struct {
    uint16 previous_task, __previous_task_reserved;
    uint32 esp0;
    uint16 ss0, __ss0_reserved;
    uint32 esp1;
    uint16 ss1, __ss1_reserved;
    uint32 esp2;
    uint16 ss2, __ss2_reserved;
    uint32 cr3;
    uint32 eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint16 es, __es_reserved;
    uint16 cs, __cs_reserved;
    uint16 ss, __ss_reserved;
    uint16 ds, __ds_reserved;
    uint16 fs, __fs_reserved;
    uint16 gs, __gs_reserved;
    uint16 ldt_selector, __ldt_sel_reserved;
    uint16 debug_flag, io_map;
} __attribute__((packed)) tss;

/* ========================================================================= */

extern void tss_flush();
uint32 tss_init();
uint32 tss_set_esp0(void * esp0);
uint32 tss_set_ss0(uint32 ss0);
tss * tss_get();

/* ========================================================================= */
