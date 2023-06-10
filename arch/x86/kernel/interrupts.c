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
        vga_printf("[isr] ISR called! Code: %d", registers->int_num);    
        if(registers->int_num == 0)
            vga_printf("[isr] Interrupt 0: Divide by Zero");
        if(registers->int_num == 1)
            vga_printf("[isr] Interrupt 1: Single Step");
        if(registers->int_num == 2)
            vga_printf("[isr] Interrupt 2: Non-Maskable (NMI)");
        if(registers->int_num == 3)
            vga_printf("[isr] Interrupt 3: Breakpoint");
        if(registers->int_num == 4)
            vga_printf("[isr] Interrupt 4: Overflow Trap");
        if(registers->int_num == 5)
            vga_printf("[isr] Interrupt 5: Bound Range Exceeded");
        if(registers->int_num == 6)
            vga_printf("[isr] Interrupt 6: Invalid Opcode");
        if(registers->int_num == 7)
            vga_printf("[isr] Interrupt 7: Coprocessor Not Available");
        if(registers->int_num == 8)
            vga_printf("[isr] Interrupt 8: Double Fault Exception");
        if(registers->int_num == 9)
            vga_printf("[isr] Interrupt 9: Coprocessor Segment Overrun");
        if(registers->int_num == 10)
            vga_printf("[isr] Interrupt 10: Invalid Task State Segment (TSS)");
        if(registers->int_num == 11)
            vga_printf("[isr] Interrupt 11: Segment Not Present");
        if(registers->int_num == 12)
            vga_printf("[isr] Interrupt 12: Stack Exception");
        if(registers->int_num == 13)
            vga_printf("[isr] Interrupt 13: General Protection Exception");
        if(registers->int_num == 14)
            vga_printf("[isr] Interrupt 14: Page Fault");

        task * current_task = task_get_current();

        vga_printf("[isr] Error Code: %d", registers->error_code);
        vga_printf("Task:       %s", current_task->name);
        vga_printf("Registers");
        vga_printf("---------");
        vga_printf("EAX:        0x%x | EBX:        0x%x", registers->eax, registers->ebx);
        vga_printf("ECX:        0x%x | EDX:        0x%x", registers->ecx, registers->edx);
        vga_printf("ESI:        0x%x | EDI:        0x%x", registers->esi, registers->edi);
        vga_printf("EIP:        0x%x | ESP:        0x%x", registers->eip, registers->esp);
        vga_printf("CS:         0x%x", registers->cs);
        vga_printf("EFLAGS:     0x%x", registers->eflags);

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

            vga_printf("Fault at:   0x%x", cr2);

            if(present == 1) {
                vga_printf("The fault was caused by a page-level protection violation.");
            } else {
                vga_printf("The fault was caused by a non-present page.");
            }
            if(write) {
                vga_printf("The access causing the fault was a write.");
            } else {
                vga_printf("The access causing the fault was a read.");
            }
            if(user) {
                vga_printf("A user-mode access caused the fault.");
            } else {
                vga_printf("A supervisor-mode access caused the fault.");
            }
            if(rsvd_write) {
                vga_printf("The fault was caused by a reserved bit set to 1 in some page entry.");
            }
            if(ins_fetch) {
                vga_printf("The fault was caused by an instruction fetch.");
            }
            if(pkey) {
                vga_printf("There was a protection-key violation.");
            }
            if(shadow_stack) {
                vga_printf("The fault was caused by a shadow-stack access.");
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
