#include <rotary/logging.h>

volatile atomic_flag print_lock = ATOMIC_FLAG_INIT;

/* ========================================================================= */

//TODO: make this not bad
void _printk(uint8 level, char * tag, char * message) {


    char final_message[512];
    char vga_message[512];
    char final_tag[16];
    memset(&final_message, 0, sizeof(final_message));
    memset(&final_tag, 0, sizeof(final_tag));
    memset(&vga_message, 0, sizeof(vga_message));

    // Format our message for the serial log
    sprintf(final_message, "[0.000][%s] %s", tag, message);

    // Format our message for VGA output
    char * vga_ptr = vga_message;
    strncpy(vga_message, message, strlen(message) + 1);

    // Replace any newline characters with null terminators for now
    while(*vga_ptr != '\0') {
        if(*vga_ptr == '\n') {
            *vga_ptr = '\0';
        }
        vga_ptr++;
    }

    // Take exclusive control of printing to video output and serial
    lock(&print_lock);

    if(level >= LOG_INFO) {
        // Write this message to the screen
        vga_printf(vga_message);

        // Write this to the serial output
        serial_write_line(IO_PORT_SERIAL_COM1, final_message);
    }

    if(level <= LOG_DEBUG) {
        // Write this to the serial output
        serial_write_line(IO_PORT_SERIAL_COM2, final_message);
    }

    // Release control so others can print
    unlock(&print_lock);
}

/* ========================================================================= */
