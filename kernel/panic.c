#include <rotary/panic.h>

void kpanic(char * title, char * message, isr_registers * regs) {

    char line[512];

    vga_clear_screen(VGA_COLOUR_BLUE);
    vga_overwrite_line("Kernel Panic", VGA_COLOUR_WHITE, VGA_COLOUR_BLUE, 3);
    vga_overwrite_line("---------------------------------------", VGA_COLOUR_WHITE, VGA_COLOUR_BLUE, 4);

    sprintf(line, "EIP:  0x%x", regs->eip);
    vga_overwrite_line(line, VGA_COLOUR_WHITE, VGA_COLOUR_BLUE, 5);
    debug_break();
}
