#include <rotary/shell.h>

char shell_prompt[] = "shell> ";
char input_buffer[256] = "\0";
void * command_handlers[256];

extern void * heap_start_vaddr;

/* ========================================================================= */
/*  Initialisation                                                           */
/* ========================================================================= */

void shell_init() {
    vga_scroll_up(1, TOP_LINE);
    driver_keyboard_register_handler((uint32)&shell_keyboard_handler);
    shell_print_prompt();
    while(true) { }
}

/* ========================================================================= */
/*  User Interface                                                           */
/* ========================================================================= */

void shell_print_prompt() {
    vga_overwrite_line("shell> ", VGA_COLOUR_WHITE, VGA_COLOUR_BLACK, 24);
    vga_set_cursor(7, 24);
}

/* ========================================================================= */
/*  Keyboard and Input Handling                                              */
/* ========================================================================= */

void shell_delete_from_input_buffer() {
    input_buffer[strlen(input_buffer)] = '\0';
    input_buffer[strlen(input_buffer)-1] = '\0';
}

/* ========================================================================= */

void shell_clear_input_buffer() {
    memset((void*)&input_buffer, 0, sizeof(input_buffer));
}

/* ========================================================================= */

void shell_keyboard_handler(uchar key) {
    // If the key is Enter, submit the command and scroll existing lines up
    if(key == KEY_ENTER) {
        vga_scroll_up(1, TOP_LINE);
        vga_state.cursor_x = 0;
        shell_process_command(input_buffer);
        shell_clear_input_buffer();
        shell_print_prompt();
        return;
    }

    if(key == KEY_BACKSPACE) {
        shell_delete_from_input_buffer();
        vga_delete_char_at_cursor();
        return;
    }

    if(key != 0x00) {
        *(&key + 1) = '\0';
        strcat(input_buffer, (char*)&key);
        vga_print_char_at_cursor(key);
    }

}

/* ========================================================================= */
/*  Command Handling                                                         */
/* ========================================================================= */

extern uint32 t1_counter;

void banana() {
    //task_exit_current();
    printk(LOG_DEBUG, "BANANA initialised\n");
    while(true) { }
}

void shell_process_command(char * command) {

    if(strcmp(command, "tss") != 0) {
        tss_set_esp0((void*)0xDEADBEEF);
        tss_set_ss0(0x10);
        //tss_flush();
        return;
    }

    if(strcmp(command, "banana") != 0) {
        task_create("banana", TASK_USERMODE, &banana, TASK_STATE_WAITING);
        return;
    }

    if(strcmp(command, "testkill") != 0) {
        task_kill(2);
        return;
    }

    //TODO: temp, replace with command handlers in separate files
    if(strcmp(command, "gdt") != 0) {
        uint32 gdt_addr;
        __asm__("sgdt %0" : "=m" (gdt_addr));
        vga_printf("GDT addr: %x", gdt_addr);
        return;
    }

    union CR0 {
      unsigned long val;
      struct {
        unsigned long protection_enable:1;
        unsigned long monitor_coprocessor:1;
        unsigned long emulation:1;
        unsigned long task_switched:1;
        unsigned long extension_type:1;
        unsigned long numeric_error:1;
        unsigned long write_protect:1;
        unsigned long alignment_mask:1;
        unsigned long not_write_through:1;
        unsigned long cache_disable:1;
        unsigned long paging:1;
        unsigned long reserved:20;
      } fields;
    };

    if(strcmp(command, "cr0") != 0) {
        union CR0 cr0;
        __asm__ __volatile__("movl %%cr0, %0" : "=r"(cr0.val));
        vga_printf("CRO:");
        vga_printf(" Protected Mode:       %d", cr0.fields.protection_enable);
        vga_printf(" Monitor Co-Processor: %d", cr0.fields.monitor_coprocessor);
        vga_printf(" Emulation:            %d", cr0.fields.emulation);
        vga_printf(" Task Switched:        %d", cr0.fields.task_switched);
        vga_printf(" Extension Type:       %d", cr0.fields.extension_type);
        vga_printf(" Numeric Error:        %d", cr0.fields.numeric_error);
        vga_printf(" Write Protect:        %d", cr0.fields.write_protect);
        vga_printf(" Alignment Mask:       %d", cr0.fields.alignment_mask);
        vga_printf(" Not-Write Through:    %d", cr0.fields.not_write_through);
        vga_printf(" Cache Disable:        %d", cr0.fields.cache_disable);
        vga_printf(" Paging:               %d", cr0.fields.paging);
        return;
    }

    if(strcmp(command, "lol2") != 0) {
        asm("int $32");
        return;
    }

    if(strcmp(command, "clear") != 0 || strcmp(command, "cls") != 0) {
        for(uint32 i = 4; i < VGA_HEIGHT; i++) {
            vga_clear_line(i);
        }
        return;
    }

    if(strcmp(command, "build") != 0) {
        vga_printf("Kernel built: %s %s", __DATE__, __TIME__);
        return;
    }

    if(strcmp(command, "tasks") != 0) {
        task_print();
        return;
    }

    if(strcmp(command, "pmm") != 0) {
        pmm_print();
        return;
    }

    if(strcmp(command, "vmm") != 0) {
        vmm_print();
        return;
    }

    if(strcmp(command, "die") != 0) {
        char * die = (char*)0xB0000000;
        printk(LOG_DEBUG, "die: 0x%x\n", *die);
        return;
    }

    if(strcmp(command, "heap") != 0) {
        block_header * block = (block_header*)heap_start_vaddr;
        while(block->size != 0) {
            uint32 trailer_addr = (uint32)block + sizeof(block_header) + block->size;
            printk(LOG_INFO, "addr %x sz %d us %d pr %x nx %x tr %x\n", block, block->size, block->used,
                    block->prev_free_block, block->next_free_block, trailer_addr);
            block = (block_header*)((char*)block + sizeof(block_header) + block->size + sizeof(block_trailer));

        }
        return;
    }

    vga_printf("Unknown command: %s", command);
}

/* ========================================================================= */

void shell_register_handler(char * command, void * handler) {
    
}

/* ========================================================================= */

