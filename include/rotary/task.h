/* ========================================================================= */
/* Tasks and Processes                                                       */
/* ========================================================================= */
#pragma once

#include <rotary/types.h>
#include <rotary/interrupts.h>
#include <rotary/logging.h>
#include <rotary/vga.h>
#include <rotary/paging.h>
#include <rotary/vmm.h>

/* ========================================================================= */

#undef  LOG_TAG
#define LOG_TAG "scheduler"

// Task States
#define TASK_STATE_EMPTY        0
#define TASK_STATE_RUNNING      1
#define TASK_STATE_WAITING      2

#define TASK_MAX                8

#define TASK_KERNEL             0
#define TASK_USERMODE           1

#define TASK_STACK_SIZE         8192

/* ========================================================================= */

// Used to store information about a task.
typedef struct {
    uint32 id;
    uint32 kernel_esp;
    uint32 kernel_ebp;
    uint32 cr3;         
    uint32 state;       
    uint32 ticks;
    page_directory_entry * page_directory;
    char name[16];
} task;

// Used to access stored registers and return values on the stack
// during a task context switch.
typedef struct {
    uint32 edi; // Stored by task_switch
    uint32 esi; // Stored by task_switch
    uint32 ebx; // Stored by task_switch
    uint32 ebp; // Stored by task_switch
    uint32 eip; // Return value stored during cdecl call to task_switch
} task_context;

/* ========================================================================= */

uint32 task_init();
task * task_create(char * name, uint32 type, void * start_addr);
task * task_get_current();
void   task_schedule();
void   task_print();

/* ========================================================================= */
