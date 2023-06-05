#include <rotary/task.h>

extern void task_switch(task * current_task, task * next_task);
extern void isr_exit();

// ========================================================================= //
// Variables                                                                 //
// ========================================================================= // 

uint32 last_task_id = 1;
uint8  scheduler_ready = 0;
task   tasks[TASK_MAX];
task * current_task;

// ========================================================================= //
// Test Functions                                                            //
// ========================================================================= // 

void test1() {
    while(true) {
        //char * die = 0xB0000000;
        //printk(LOG_DEBUG, "die: 0x%x\n", *die);
    }
}

void test2() {
    while(true) { }
}

// ========================================================================= //
// Functions                                                                 //
// ========================================================================= // 

// Create our initial idle task for the current thread of execution, and enable the scheduler
uint32 task_init() {
    // Our initial task will continue the execution of kernel_main(), as when we
    // switch to a new task the current EIP/ESP will be stored in the tasks[0] struct.
    printk(LOG_DEBUG, "Creating initial task\n");
    task * idle_task = &tasks[0];
    idle_task->state = TASK_STATE_RUNNING;
    strncpy(idle_task->name, "kernel_idle", 16);
    current_task = idle_task;

    task_create("test1", TASK_KERNEL, &test1);
    task_create("test2", TASK_KERNEL, &test2);
    
    // Enable the scheduler (otherwise PIT interrupts may trigger it before it's ready)
    printk(LOG_DEBUG, "Enabling scheduler\n");
    scheduler_ready = 1;

    return 1;
}

// ========================================================================= // 

task * task_create(char * name, uint32 type, void * start_addr) {
    printk(LOG_DEBUG, "Creating task %d (name: %s addr: 0x%x)\n", last_task_id, name, start_addr);

    uint32 task_id = last_task_id;
    tasks[task_id].id         = task_id;
    strncpy(tasks[task_id].name, name, 16);

    void * kernel_stack = kmalloc(TASK_STACK_SIZE);
    if((uint32)kernel_stack == KMALLOC_FAIL) {
        printk(LOG_DEBUG, "Failed to allocate memory for the kernel stack!\n");
        return (void*)-1;
    }

    tasks[task_id].kernel_esp = (uint32)(kernel_stack + TASK_STACK_SIZE);
    tasks[task_id].kernel_ebp = (uint32)(kernel_stack + TASK_STACK_SIZE);
    printk(LOG_DEBUG, "Task kernel stack allocated at 0x%x\n", kernel_stack);

    // Set-up the new kernel stack for the task. The kernel stack will need to contain
    // null-initialised register values that will be popped off the stack by the task
    // switching routine.

    // The return address saved prior to calling task_switch() will be overwritten with
    // a call to isr_exit(), which will restore the following registers just like the end
    // of any other ISR. As a result, the EIP is where our task will start.
    tasks[task_id].kernel_esp -= sizeof(isr_registers);
    isr_registers * registers = (isr_registers*)tasks[task_id].kernel_esp;
    memset(registers, 0, sizeof(isr_registers));
    registers->eflags = 0x200;
    registers->eip = (uint32)start_addr;
    registers->cs = 0x08;

    // This struct stores registers that are saved and restored by task_switch()
    // EDI->EBP are restored via pops, EIP is restored by ret
    tasks[task_id].kernel_esp -= sizeof(task_context);
    task_context * context = (task_context*)tasks[task_id].kernel_esp;
    memset(context, 0, sizeof(task_context));
    context->edi = 0;
    context->esi = 0;
    context->ebx = 0;
    context->ebp = 0;
    context->eip = (uint32)&isr_exit;

    tasks[task_id].state = TASK_STATE_WAITING;

    last_task_id++;
    return &tasks[task_id];
}

// ========================================================================= // 

task * task_get_current() {
    return current_task;
}

// ========================================================================= // 

void task_schedule() {
    // Don't allow task switching before everything's set up
    if(!scheduler_ready)
        return;

    // Count CPU ticks for this task
    current_task->ticks += 1;

    // Identify the next task, naive implementation for now
    uint32 next_task_id = current_task->id + 1;
    if(next_task_id == 256) next_task_id = 0;

    while(tasks[next_task_id].state == TASK_STATE_EMPTY) {
        next_task_id += 1;
        if(next_task_id == 9) next_task_id = 0;
    }

    task * prev = current_task;
    task * next = &tasks[next_task_id];
    current_task = next;

    // Update states
    prev->state = TASK_STATE_WAITING;
    next->state = TASK_STATE_RUNNING;

    task_switch(prev, next);
}

// ========================================================================= // 

void task_print() {
    printk(LOG_DEBUG, "Task List\n");
    printk(LOG_DEBUG, "-----------------------------\n");
    for(uint32 i = 0; i < TASK_MAX; i++) {
        char state[16];
        memset(&state, 0x00, sizeof(state));

        if(tasks[i].state == TASK_STATE_EMPTY)
            sprintf(state, "%s", "EMPTY");
        if(tasks[i].state == TASK_STATE_RUNNING)
            sprintf(state, "%s", "RUNNING");
        if(tasks[i].state == TASK_STATE_WAITING)
            sprintf(state, "%s", "WAITING");
        if(tasks[i].state != TASK_STATE_EMPTY) {
            printk(LOG_DEBUG, "[%d] name:   %s\n", i, tasks[i].name);
            printk(LOG_DEBUG, "     state:  %s\n", state);
            printk(LOG_DEBUG, "     k_esp:  0x%x\n", tasks[i].kernel_esp);
            printk(LOG_DEBUG, "     k_ebp:  0x%x\n", tasks[i].kernel_ebp);
            printk(LOG_DEBUG, "     stack:  %d\n", tasks[i].kernel_ebp - tasks[i].kernel_esp);
            printk(LOG_DEBUG, "     cr3:    0x%x\n", tasks[i].cr3);
            printk(LOG_DEBUG, "     ticks:  %d\n", tasks[i].ticks);
        }
    }
    printk(LOG_DEBUG, "\n");
}

// ========================================================================= // 
