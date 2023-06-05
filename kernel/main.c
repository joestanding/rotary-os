/* ========================================================================= */
/* Rotary OS                                                                 */
/* ========================================================================= */

#include <rotary/main.h>

extern uint32 KERNEL_PHYS_START;
extern uint32 KERNEL_PHYS_END;
extern uint32 KERNEL_STACK_BOTTOM;
extern uint32 KERNEL_STACK_TOP;

void kernel_main(uint32 mboot_magic, multiboot_info_t * mboot_info) {

    // Initialise serial as early as we can for the sake of debug output
    printk(LOG_DEBUG, "Initialising serial output on COM1..\n");
    serial_init();

    vga_clear_screen(VGA_COLOUR_BLACK);

    // Banner
    vga_overwrite_line(KERNEL_NAME, VGA_COLOUR_RED, VGA_COLOUR_BLACK, 0);
    vga_overwrite_line(KERNEL_VERSION, VGA_COLOUR_WHITE, VGA_COLOUR_BLACK, 1);
    vga_print_separator(VGA_COLOUR_CYAN, 2);

    printk(LOG_DEBUG, "Kernel entry..\n");
    printk(LOG_DEBUG, "KERNEL_PHYS_START:  0x%x\n", &KERNEL_PHYS_START);
    printk(LOG_DEBUG, "KERNEL_PHYS_END:    0x%x\n", &KERNEL_PHYS_END);
    printk(LOG_DEBUG, "KERNEL_STACK_BOTTOM:  0x%x\n", &KERNEL_STACK_BOTTOM);
    printk(LOG_DEBUG, "KERNEL_STACK_TOP:     0x%x\n", &KERNEL_STACK_TOP);

    printk(LOG_DEBUG, "Parsing Multiboot structures..\n");
    if(!multiboot_parse(mboot_magic, (multiboot_info_t*)mboot_info)) {
        return;
    }

    printk(LOG_DEBUG, "Setting up GDT..\n");
    if(!gdt_init()) {
        return;
    }

    printk(LOG_DEBUG, "Assigning IDT gates..\n");
    if(!idt_init()) {
        return;
    }

    printk(LOG_DEBUG, "Loading IDT..\n");
    if(!idt_load()) {
        return;
    }

    printk(LOG_DEBUG, "Configuring legacy PIC..\n");
    if(!pic_init(PIC_MASTER_OFFSET, PIC_SLAVE_OFFSET)) {
        return;
    }

    printk(LOG_DEBUG, "Initialising timer..\n");
    if(!timer_init()) {
        return;
    }
    
    printk(LOG_DEBUG, "Initialising keyboard driver..\n");
    if(!driver_keyboard_init()) {
        return;
    }

    printk(LOG_DEBUG, "Enabling interrupts..\n");
    if(!enable_hardware_interrupts()) {
        return;
    }

    printk(LOG_DEBUG, "Initialising physical memory manager..\n");
    if(!pmm_init()) {
        return;
    }

    printk(LOG_DEBUG, "Initialising virtual memory manager..\n");
    if(!vmm_init()) {
        return;
    }

    if(mboot_info->flags & MULTIBOOT_INFO_CMDLINE) {
        if(strcmp((char*)mboot_info->cmdline, "test") != 0) {
            printk(LOG_DEBUG, "\n--- STARTING TESTS ---\n");
            test_init();
            debug_break();
        }
    }

    printk(LOG_DEBUG, "Initialising task scheduler..\n");
    if(!task_init()) {
        return;
    }

    printk(LOG_DEBUG, "\n");
    printk(LOG_DEBUG, "Kernel initialisation complete!\n");
    printk(LOG_DEBUG, "\n");
    
    task_create("shell", TASK_KERNEL, &shell_init);
    
    while(true) { }
}


