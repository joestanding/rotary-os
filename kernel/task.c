#include <rotary/task.h>

extern void task_switch(task * current_task, task * next_task);
extern void isr_exit();

// ========================================================================= //
// Variables                                                                 //
// ========================================================================= // 

volatile atomic_flag task_lock = ATOMIC_FLAG_INIT;
uint32 last_task_id = 1;
uint8  scheduler_ready = 0;
task   tasks[TASK_MAX];
task * current_task;

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

    // Enable the scheduler (otherwise PIT interrupts may trigger it before it's ready)
    printk(LOG_DEBUG, "Enabling scheduler\n");
    scheduler_ready = 1;

    return 1;
}

// ========================================================================= // 

task * task_create(char * name, uint32 type, void * start_addr, uint32 state) {
    printk(LOG_DEBUG, "task_create(): Creating task %d (name: %s addr: 0x%x)\n", last_task_id, name, start_addr);

    if(state != TASK_STATE_PAUSED && state != TASK_STATE_WAITING) {
        printk(LOG_DEBUG, "task_create(): Invalid starting state, can only be WAITING or PAUSED!\n");
        return (void*)1;
    }

    // Take exclusive control of the task structure
    lock(&task_lock);

    uint32 task_id = last_task_id;
    memset(&tasks[task_id], 0, sizeof(task));
    tasks[task_id].id = task_id;
    strncpy(tasks[task_id].name, name, 16);

    // Allocate memory from the kernel heap
    printk(LOG_TRACE, "task_create(): Allocating kernel stack with kmalloc\n");
    void * kernel_stack = kmalloc(TASK_STACK_SIZE);
    if((uint32)kernel_stack == KMALLOC_FAIL) {
        printk(LOG_DEBUG, "task_create(): Failed to allocate memory for the kernel stack!\n");
        return (void*)0;
    }

    // Store some information about the kernel stack allocation
    tasks[task_id].stack_frame = kernel_stack;
    tasks[task_id].kernel_esp = kernel_stack + TASK_STACK_SIZE;
    tasks[task_id].kernel_ebp = kernel_stack + TASK_STACK_SIZE;
    printk(LOG_DEBUG, "task_create(): Task kernel stack allocated at 0x%x\n", kernel_stack);

    if(type == TASK_KERNEL) {
        printk(LOG_DEBUG, "task_create(): Task is a kernel task, assigning kernel PDE\n");
        tasks[task_id].cr3 = (uint32)VIR_TO_PHY(vmm_kernel_pd);
    } else {
        printk(LOG_DEBUG, "task_create(): Task is a user task, creating new PDE\n");
        // Create a new page directory for the task
        page_directory_entry * page_directory = (page_directory_entry*)PHY_TO_VIR(pmm_alloc_frame());
        paging_init_directory(page_directory);
        // Set up some initial mappings including mapping the kernel in
        vmm_map_kernel(page_directory);
        tasks[task_id].cr3 = (uint32)VIR_TO_PHY(page_directory);
        printk(LOG_DEBUG, "task_create(): Task page directory allocated at 0x%x\n", page_directory);
    }

    // Set-up the new kernel stack for the task. The kernel stack will need to contain
    // null-initialised register values that will be popped off the stack by the task
    // switching routine.

    // The return address saved prior to calling task_switch() will be overwritten with
    // a call to isr_exit(), which will restore the following registers just like the end
    // of any other ISR. As a result, the EIP is where our task will start.
    tasks[task_id].kernel_esp -= sizeof(isr_registers);
    printk(LOG_TRACE, "task_create(): isr_registers struct is at 0x%x\n", tasks[task_id].kernel_esp);
    isr_registers * registers = (isr_registers*)tasks[task_id].kernel_esp;
    memset(registers, 0, sizeof(isr_registers));
    registers->eflags = 0x200;
    registers->eip = (uint32)start_addr;
    registers->cs = 0x08;

    // This struct stores registers that are saved and restored by task_switch()
    // EDI->EBP are restored via pops, EIP is restored by ret
    tasks[task_id].kernel_esp -= sizeof(task_context);
    printk(LOG_TRACE, "task_create(): context struct is at 0x%x\n", tasks[task_id].kernel_esp);
    task_context * context = (task_context*)tasks[task_id].kernel_esp;
    memset(context, 0, sizeof(task_context));
    context->edi = 0;
    context->esi = 0;
    context->ebx = 0;
    context->ebp = 0;
    context->eip = (uint32)&isr_exit;

    // Set the state of the task so it's ready to be scheduled
    tasks[task_id].state = state;

    last_task_id++;

    // Relinquish control of the task structure
    unlock(&task_lock);

    printk(LOG_TRACE, "task_create(): Created task '%s' (PID: %d) with attributes:\n", tasks[task_id].name, task_id);
    printk(LOG_TRACE, "               EIP:  0x%x\n", registers->eip);
    printk(LOG_TRACE, "               ESP:  0x%x\n", tasks[task_id].kernel_esp);

    return &tasks[task_id];
}

/* ========================================================================= */

uint32 task_kill(uint32 task_id) {
    if(task_id < 0 || task_id > TASK_MAX) {
        printk(LOG_DEBUG, "task_kill(): Task ID %d is out of bounds!\n", task_id);
        return 0;
    }

    if(task_id == 0) {
        printk(LOG_DEBUG, "task_kill(): You cannot kill the idle process (ID 0)!\n", task_id);
        return 0;
    }

    task * task_tk = &tasks[task_id];
    if(task_tk->state == TASK_STATE_EMPTY) {
        printk(LOG_DEBUG, "task_kill(): Task ID %d is not an existing task!\n", task_id);
        return 0;
    }

    if(task_tk->state != TASK_STATE_RUNNING &&
       task_tk->state != TASK_STATE_WAITING &&
       task_tk->state != TASK_STATE_PAUSED) {
        printk(LOG_DEBUG, "task_kill(): Task ID %d is not in a killable state!\n", task_id);
        return 0;
    }

    printk(LOG_DEBUG, "task_kill(): Killing task '%s' (ID: %d)..\n", task_tk->name, task_id);

    // Update state so we don't schedule this task anymore. The OS will
    // decide when to clear its resources such as allocated memory.
    task_tk->state = TASK_STATE_KILLED;
   
    printk(LOG_DEBUG, "task_kill(): Marked task ID %d as killed\n", task_id);
    return 1;
}

/* ========================================================================= */

uint32 task_purge(uint32 task_id) {
    if(task_id < 0 || task_id > TASK_MAX) {
        printk(LOG_DEBUG, "task_purge(): Task ID %d is out of bounds!\n", task_id);
        return 0;
    }

    if(task_id == 0) {
        printk(LOG_DEBUG, "task_kill(): You cannot purge the idle process (ID 0)!\n", task_id);
        return 0;
    }

    task * task_tk = &tasks[task_id];
    if(task_tk->state != TASK_STATE_KILLED) {
        printk(LOG_DEBUG, "task_kill(): Task ID %d is not a killed task!\n", task_id);
        return 0;
    }

    printk(LOG_DEBUG, "task_purge(): Purging task ID %d\n", task_id);

    // Reset this tasks's state
    task_tk->state = TASK_STATE_EMPTY;

    // Free memory allocated for this task
    if(!kfree(task_tk->stack_frame)) {
        printk(LOG_DEBUG, "task_purge(): Failed to kfree() task's stack at 0x%x!\n",
                task_tk->stack_frame);
    }

    // Reset the rest of the task struct
    task_tk->kernel_esp = (void*)NULL;
    task_tk->kernel_ebp = (void*)NULL;
    task_tk->stack_frame = (void*)NULL;
    task_tk->cr3 = NULL;
    task_tk->ticks = NULL;

    printk(LOG_DEBUG, "task_kill(): Purged task ID %d\n", task_id);

    return 1;
}

/* ========================================================================= */

uint32 task_exit_current() {
    printk(LOG_DEBUG, "task_exit(): Exiting task '%s'\n", current_task->name);
    lock(&task_lock);
    current_task->state = TASK_STATE_KILLED;
    unlock(&task_lock);
    return 1;
}

/* ========================================================================= */

task * task_get_current() {
    return current_task;
}

/* ========================================================================= */

void task_schedule() {
    // Don't allow task switching before everything's set up
    if(!scheduler_ready)
        return;

    // Count CPU ticks for this task
    current_task->ticks += 1;

    // TODO: Review whether this is actually a good place to put this
    for(uint32 task_id = 0; task_id < TASK_MAX; task_id++) {
        if(tasks[task_id].state == TASK_STATE_KILLED) {
            printk(LOG_DEBUG, "task_schedule(): Found KILLED task (%d) awaiting purge\n", task_id);
            task_purge(task_id);
        }
    }

    // Identify the next task, naive implementation for now
    uint32 next_task_id = current_task->id + 1;
    if(next_task_id == 256) next_task_id = 0;

    // TODO: Replace
    while(tasks[next_task_id].state == TASK_STATE_EMPTY ||
          tasks[next_task_id].state == TASK_STATE_PAUSED ||
          tasks[next_task_id].state == TASK_STATE_KILLED) {
        next_task_id += 1;
        if(next_task_id == 9) next_task_id = 0;
    }

    task * prev = current_task;
    task * next = &tasks[next_task_id];
    current_task = next;

    // Update states
    if(prev->state == TASK_STATE_RUNNING) {
        prev->state = TASK_STATE_WAITING;
    }
    next->state = TASK_STATE_RUNNING;

    // Call our assembly routine to conduct the actual task switch
    task_switch(prev, next);
}

/* ========================================================================= */

void task_print() {
    printk(LOG_INFO, "Task List\n");
    printk(LOG_INFO, "-----------------------------\n");
    for(uint32 i = 0; i < TASK_MAX; i++) {
        char state[16];
        memset(&state, 0x00, sizeof(state));

        if(tasks[i].state == TASK_STATE_EMPTY)
            sprintf(state, "%s", "EMPTY");
        if(tasks[i].state == TASK_STATE_RUNNING)
            sprintf(state, "%s", "RUNNING");
        if(tasks[i].state == TASK_STATE_WAITING)
            sprintf(state, "%s", "WAITING");
        if(tasks[i].state == TASK_STATE_PAUSED)
            sprintf(state, "%s", "PAUSED");
        if(tasks[i].state == TASK_STATE_KILLED)
            sprintf(state, "%s", "KILLED");
        if(tasks[i].state != TASK_STATE_EMPTY) {
            printk(LOG_INFO, "[%d]   name:   %s   (%s)\n", i, tasks[i].name, state);
            printk(LOG_INFO, "      k_esp:  0x%x\n", tasks[i].kernel_esp);
            printk(LOG_INFO, "      k_ebp:  0x%x\n", tasks[i].kernel_ebp);
            printk(LOG_INFO, "      stack:  %d\n", tasks[i].kernel_ebp - tasks[i].kernel_esp);
            printk(LOG_INFO, "      cr3:    0x%x\n", tasks[i].cr3);
            printk(LOG_INFO, "      ticks:  %d\n", tasks[i].ticks);
        }
    }
    printk(LOG_INFO, "\n");
}

/* ========================================================================= */
