#include <rotary/interrupts.h>

uint32 count = 0;

/* ========================================================================= */
/*  Enabling and Disabling Interrupts                                        */
/* ========================================================================= */

uint32 enable_hardware_interrupts() {
    asm("sti");
    return 1;
}

/* ========================================================================= */

void disable_hardware_interrupts() {
    asm("cli");
}

/* ========================================================================= */
/*  Dynamic Interrupt Handlers (registered by drivers, etc.)                 */
/* ========================================================================= */

uint32 interrupt_handlers[256];
void interrupt_init() {
}

/* ========================================================================= */

void register_interrupt_handler(uint32 int_num, void * handler_addr) {
    printk(LOG_DEBUG, "Registered handler (0x%x) for int. %d\n", handler_addr, int_num);
    interrupt_handlers[int_num] = (uint32)handler_addr;
}

/* ========================================================================= */
/*  Generic Interrupt Handlers                                               */
/* ========================================================================= */

void isr_handler(isr_registers * registers) {
    if(registers->int_num == INT_PIT) {
        pic_send_eoi(registers->int_num);
    }

    // If a handler for this interrupt is registered, call it
    if(interrupt_handlers[registers->int_num] != 0x00000000) {
        func * handler = (func*)interrupt_handlers[registers->int_num];
        handler(registers);
    }

    // Print some debugging information for exceptions raised by the CPU
    if(registers->int_num >= 0 && registers->int_num < 32) {
        printk(LOG_INFO, "[isr] ISR called! Code: %d\n", registers->int_num);    
        if(registers->int_num == 0)
            printk(LOG_INFO, "[isr] Interrupt 0: Divide by Zero\n");
        if(registers->int_num == 1)
            printk(LOG_INFO, "[isr] Interrupt 1: Single Step\n");
        if(registers->int_num == 2)
            printk(LOG_INFO, "[isr] Interrupt 2: Non-Maskable (NMI)\n");
        if(registers->int_num == 3)
            printk(LOG_INFO, "[isr] Interrupt 3: Breakpoint\n");
        if(registers->int_num == 4)
            printk(LOG_INFO, "[isr] Interrupt 4: Overflow Trap\n");
        if(registers->int_num == 5)
            printk(LOG_INFO, "[isr] Interrupt 5: Bound Range Exceeded\n");
        if(registers->int_num == 6)
            printk(LOG_INFO, "[isr] Interrupt 6: Invalid Opcode\n");
        if(registers->int_num == 7)
            printk(LOG_INFO, "[isr] Interrupt 7: Coprocessor Not Available\n");
        if(registers->int_num == 8)
            printk(LOG_INFO, "[isr] Interrupt 8: Double Fault Exception\n");
        if(registers->int_num == 9)
            printk(LOG_INFO, "[isr] Interrupt 9: Coprocessor Segment Overrun\n");
        if(registers->int_num == 10)
            printk(LOG_INFO, "[isr] Interrupt 10: Invalid Task State Segment (TSS)\n");
        if(registers->int_num == 11)
            printk(LOG_INFO, "[isr] Interrupt 11: Segment Not Present\n");
        if(registers->int_num == 12)
            printk(LOG_INFO, "[isr] Interrupt 12: Stack Exception\n");
        if(registers->int_num == 13)
            printk(LOG_INFO, "[isr] Interrupt 13: General Protection Exception\n");
        if(registers->int_num == 14)
            printk(LOG_INFO, "[isr] Interrupt 14: Page Fault\n");

        //task * current_task = task_get_current();

        printk(LOG_INFO, "[isr] Error Code: %d\n", registers->error_code);
        //printk(LOG_INFO, "Task:       %s\n", current_task->name);
        printk(LOG_INFO, "Registers\n");
        printk(LOG_INFO, "---------\n");
        printk(LOG_INFO, "EAX:        0x%x | EBX:        0x%x\n", registers->eax, registers->ebx);
        printk(LOG_INFO, "ECX:        0x%x | EDX:        0x%x\n", registers->ecx, registers->edx);
        printk(LOG_INFO, "ESI:        0x%x | EDI:        0x%x\n", registers->esi, registers->edi);
        printk(LOG_INFO, "EIP:        0x%x | ESP:        0x%x\n", registers->eip, registers->esp);
        printk(LOG_INFO, "CS:         0x%x\n", registers->cs);
        printk(LOG_INFO, "EFLAGS:     0x%x\n", registers->eflags);

        if(registers->int_num == INT_PAGE_FAULT) {
            uint8 present = registers->error_code & 0x1;
            uint8 write = (registers->error_code >> 1) & 0x1;
            uint8 user = (registers->error_code >> 2) & 0x1;
            uint8 rsvd_write = (registers->error_code >> 3) & 0x1;
            uint8 ins_fetch = (registers->error_code >> 4) & 0x1;
            uint8 pkey = (registers->error_code >> 5) & 0x1;
            uint8 shadow_stack = (registers->error_code >> 6) & 0x1;
            //uint8 sgx = (registers->error_code >> 7) & 0x1;
            uint32 cr2;
            __asm__ volatile("mov %%cr2, %0" : "=r" (cr2));

            printk(LOG_INFO, "Fault at:   0x%x\n", cr2);

            if(present == 1) {
                printk(LOG_INFO, "The fault was caused by a page-level protection violation.\n");
            } else {
                printk(LOG_INFO, "The fault was caused by a non-present page.\n");
            }
            if(write) {
                printk(LOG_INFO, "The access causing the fault was a write.\n");
            } else {
                printk(LOG_INFO, "The access causing the fault was a read.\n");
            }
            if(user) {
                printk(LOG_INFO, "A user-mode access caused the fault.\n");
            } else {
                printk(LOG_INFO, "A supervisor-mode access caused the fault.\n");
            }
            if(rsvd_write) {
                printk(LOG_INFO, "The fault was caused by a reserved bit set to 1 in some page entry.\n");
            }
            if(ins_fetch) {
                printk(LOG_INFO, "The fault was caused by an instruction fetch.\n");
            }
            if(pkey) {
                printk(LOG_INFO, "There was a protection-key violation.\n");
            }
            if(shadow_stack) {
                printk(LOG_INFO, "The fault was caused by a shadow-stack access.\n");
            }
        }

        // In normal OS operation we'd handle these exceptions gracefully, but as we're early
        // in development we'll pause to debug this
        printk(LOG_DEBUG, "Pausing CPU..\n");
        debug_break();
    }

    // Send an End of Interrupt signal to the PICs, if needed.
    pic_send_eoi(registers->int_num);
}

/* ========================================================================= */
/*  Interrupt Descriptor Table                                               */
/* ========================================================================= */

idt_gate idt[256];
idt_pointer idt_ptr;

void set_idt_gate(uint32 int_num, uint32 handle_addr) {
    printk(LOG_DEBUG, "Assigned interrupt %d ISR at 0x%x\n", int_num, handle_addr);
    idt[int_num].low_offset = low_16(handle_addr);
    idt[int_num].segment_selector = 0x08;
    idt[int_num].reserved = 0x00;
    idt[int_num].high_offset = high_16(handle_addr);
    idt[int_num].attributes = 0x8E;
    // 32-bit Interrupt Gate: 0x8E (p=1, dpl=0b00, type=0b1110 => type_attributes=0b1000_1110=0x8E)
}

uint32 idt_load() {
    printk(LOG_DEBUG, "IDT addr: 0x%x, descriptor 0x%x\n", &idt, &idt_ptr);
    idt_ptr.base = (uint32)&idt;
    idt_ptr.limit = 256 * sizeof(idt_gate) - 1;
    asm("lidt (%0)" : : "r" (&idt_ptr));

    return 1;
}

/* ========================================================================= */
/*  Interrupt Descriptor Table Configuration                                 */
/* ========================================================================= */

uint32 idt_init() {
    // CPU Exceptions and Interrupts
    set_idt_gate(0, (uint32)isr0);
    set_idt_gate(1, (uint32)isr1);
    set_idt_gate(2, (uint32)isr2);
    set_idt_gate(3, (uint32)isr3);
    set_idt_gate(4, (uint32)isr4);
    set_idt_gate(5, (uint32)isr5);
    set_idt_gate(6, (uint32)isr6);
    set_idt_gate(7, (uint32)isr7);
    set_idt_gate(8, (uint32)isr8);
    set_idt_gate(9, (uint32)isr9);
    set_idt_gate(10, (uint32)isr10);
    set_idt_gate(11, (uint32)isr11);
    set_idt_gate(12, (uint32)isr12);
    set_idt_gate(13, (uint32)isr13);
    set_idt_gate(14, (uint32)isr14);
    set_idt_gate(15, (uint32)isr15);
    set_idt_gate(16, (uint32)isr16);
    set_idt_gate(17, (uint32)isr17);
    set_idt_gate(18, (uint32)isr18);
    set_idt_gate(19, (uint32)isr19);
    set_idt_gate(20, (uint32)isr20);
    set_idt_gate(21, (uint32)isr21);
    set_idt_gate(22, (uint32)isr22);
    set_idt_gate(23, (uint32)isr23);
    set_idt_gate(24, (uint32)isr24);
    set_idt_gate(25, (uint32)isr25);
    set_idt_gate(26, (uint32)isr26);
    set_idt_gate(27, (uint32)isr27);
    set_idt_gate(28, (uint32)isr28);
    set_idt_gate(29, (uint32)isr29);
    set_idt_gate(30, (uint32)isr30);
    set_idt_gate(31, (uint32)isr31);
    // External IRQs
    set_idt_gate(32, (uint32)isr32);
    set_idt_gate(33, (uint32)isr33);
    set_idt_gate(34, (uint32)isr34);
    set_idt_gate(35, (uint32)isr35);
    set_idt_gate(36, (uint32)isr36);
    set_idt_gate(37, (uint32)isr37);
    set_idt_gate(38, (uint32)isr38);
    set_idt_gate(39, (uint32)isr39);
    set_idt_gate(40, (uint32)isr40);
    set_idt_gate(41, (uint32)isr41);
    set_idt_gate(42, (uint32)isr42);
    set_idt_gate(43, (uint32)isr43);
    set_idt_gate(44, (uint32)isr44);
    set_idt_gate(45, (uint32)isr45);
    set_idt_gate(46, (uint32)isr46);
    set_idt_gate(47, (uint32)isr47);

    return 1;
}
