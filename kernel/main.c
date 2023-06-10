/* ========================================================================= */
/* Rotary OS                                                                 */
/* ========================================================================= */

#include <rotary/main.h>

extern uint32 KERNEL_PHYS_START;
extern uint32 KERNEL_PHYS_END;
extern uint32 KERNEL_STACK_BOTTOM;
extern uint32 KERNEL_STACK_TOP;

void kernel_main(uint32 mboot_magic, multiboot_info_t * mboot_info) {

    vga_clear_screen(VGA_COLOUR_BLACK);
    vga_set_cursor(0, 23);

    // Initialise serial as early as we can for the sake of debug output
    printk(LOG_INFO, "Initialising serial ports..            ");
    if(!serial_init()) {
        printk(LOG_INFO, FAIL_STR);
        return;
    }
    printk(LOG_INFO, OK_STR);

    // Banner
    vga_overwrite_line(KERNEL_NAME, VGA_COLOUR_RED, VGA_COLOUR_BLACK, 0);
    vga_print_separator(VGA_COLOUR_CYAN, 1);

    printk(LOG_DEBUG, "KERNEL_PHYS_START:  0x%x\n", &KERNEL_PHYS_START);
    printk(LOG_DEBUG, "KERNEL_PHYS_END:    0x%x\n", &KERNEL_PHYS_END);
    printk(LOG_DEBUG, "KERNEL_STACK_BOTTOM:  0x%x\n", &KERNEL_STACK_BOTTOM);
    printk(LOG_DEBUG, "KERNEL_STACK_TOP:     0x%x\n", &KERNEL_STACK_TOP);

    printk(LOG_INFO, "Parsing Multiboot structs..            ");
    if(!multiboot_parse(mboot_magic, (multiboot_info_t*)mboot_info)) {
        printk(LOG_INFO, FAIL_STR);
        return;
    }
    printk(LOG_INFO, OK_STR);

    printk(LOG_INFO, "Setting up GDT..                       ");
    if(!gdt_init()) {
        printk(LOG_INFO, FAIL_STR);
        return;
    }
    printk(LOG_INFO, OK_STR);

    printk(LOG_INFO, "Assigning IDT gates..                  ");
    if(!idt_init()) {
        printk(LOG_INFO, FAIL_STR);
        return;
    }
    printk(LOG_INFO, OK_STR);

    printk(LOG_INFO, "Loading IDT..                          ");
    if(!idt_load()) {
        printk(LOG_INFO, FAIL_STR);
        return;
    }
    printk(LOG_INFO, OK_STR);

    printk(LOG_INFO, "Configuring legacy PIC..               ");
    if(!pic_init(PIC_MASTER_OFFSET, PIC_SLAVE_OFFSET)) {
        printk(LOG_INFO, FAIL_STR);
        return;
    }
    printk(LOG_INFO, OK_STR);

    printk(LOG_INFO, "Initialising timer..                   ");
    if(!timer_init()) {
        printk(LOG_INFO, FAIL_STR);
        return;
    }
    printk(LOG_INFO, OK_STR);
    
    printk(LOG_INFO, "Initialising keyboard driver..         ");
    if(!driver_keyboard_init()) {
        printk(LOG_INFO, FAIL_STR);
        return;
    }
    printk(LOG_INFO, OK_STR);

    printk(LOG_INFO, "Enabling interrupts..                  ");
    if(!enable_hardware_interrupts()) {
        printk(LOG_INFO, FAIL_STR);
        return;
    }
    printk(LOG_INFO, OK_STR);

    printk(LOG_INFO, "Initialising physical memory manager.. ");
    if(!pmm_init()) {
        printk(LOG_INFO, FAIL_STR);
        return;
    }
    printk(LOG_INFO, OK_STR);

    printk(LOG_INFO, "Initialising virtual memory manager..  ");
    if(!vmm_init()) {
        printk(LOG_INFO, FAIL_STR);
        return;
    }
    printk(LOG_INFO, OK_STR);

    if(mboot_info->flags & MULTIBOOT_INFO_CMDLINE) {
        if(strcmp((char*)mboot_info->cmdline, "test") != 0) {
            printk(LOG_INFO, "\n--- STARTING TESTS ---\n");
            test_init();
            debug_break();
        }
    }

    printk(LOG_INFO, "Initialising task scheduler..          ");
    if(!task_init()) {
        printk(LOG_INFO, FAIL_STR);
        return;
    }
    printk(LOG_INFO, OK_STR);

    printk(LOG_INFO, "\n");
    printk(LOG_INFO, "Kernel initialisation complete!\n");

    task_create("shell", TASK_KERNEL, &shell_init, TASK_STATE_WAITING);
    
    while(true) { }
}


