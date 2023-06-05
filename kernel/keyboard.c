#include <rotary/keyboard.h>

/* ========================================================================= */

uint32 keyboard_event_handlers[256];

/* ========================================================================= */

// Initialisation function, we register a callback for the PS/2 interrupt
uint32 driver_keyboard_init() {
    register_interrupt_handler(33, &driver_keyboard_isr);
    return 1;
}

/* ========================================================================= */

// Interrupt service routine for PS/2 keyboards
void driver_keyboard_isr(isr_registers * registers) {
    uint8 keypress = io_port_in(IO_PORT_PS2_KEYBOARD);
    uchar key = driver_keyboard_translate_scan_code(keypress);
    for(uint32 i = 0; i < 256; i++) {
        if(keyboard_event_handlers[i] != 0x00) {
            keyboard_handler * kh = (keyboard_handler*)keyboard_event_handlers[i];
            kh(key);
        }
    }
}

/* ========================================================================= */

// Other OS components use this to register a callback that will be called when a key
// is pressed. For example, the command line needs to know when keys are pressed.
void driver_keyboard_register_handler(uint32 handler_addr) {
    for(uint32 i = 0; i < 256; i++ ) {
        if(keyboard_event_handlers[i] == 0x00) {
            keyboard_event_handlers[i] = handler_addr;
            return;
        }
    }
}

/* ========================================================================= */

// Convert a keyboard scan code into an ASCII character.
char driver_keyboard_translate_scan_code(uint8 scan_code) {
    switch(scan_code) {
        case 0x01:
            return 0x00;
        case 0x02:
            return 0x02;
        case 0x03:
            return '2';
        case 0x04:
            return '3';
        case 0x05:
            return '4';
        case 0x06:
            return '5';
        case 0x07:
            return '6';
        case 0x08:
            return '7';
        case 0x09:
            return '8';
        case 0x0A:
            return '9';
        case 0x0B:
            return '0';
        case 0x0C:
            return '-';
        case 0x0D:
            return '=';
        case 0x0E:
            return 0x0E;
        case 0x0F:
            return 0x0F;
        case 0x10:
            return 'q';
        case 0x11:
            return 'w';
        case 0x12:
            return 'e';
        case 0x13:
            return 'r';
        case 0x14:
            return 't';
        case 0x15:
            return 'y';
        case 0x16:
            return 'u';
        case 0x17:
            return 'i';
        case 0x18:
            return 'o';
        case 0x19:
            return 'p';
        case 0x1A:
            return '[';
        case 0x1B:
            return ']';
        case 0x1C:
            return 0x1C; // Enter
        case 0x1D:
            return 0x1D; // Left Control
        case 0x1E:
            return 'a';
        case 0x1F:
            return 's';
        case 0x20:
            return 'd';
        case 0x21:
            return 'f';
        case 0x22:
            return 'g';
        case 0x23:
            return 'h';
        case 0x24:
            return 'j';
        case 0x25:
            return 'k';
        case 0x26:
            return 'l';
        case 0x27:
            return ';';
        case 0x28:
            return '\'';
        case 0x29:
            return '`';
        case 0x2A:
            return 0x2A; // Left Shift
        case 0x2B:
            return '\\';
        case 0x2C:
            return 'z';
        case 0x2D:
            return 'x';
        case 0x2E:
            return 'c';
        case 0x2F:
            return 'v';
        case 0x30:
            return 'b';
        case 0x31:
            return 'n';
        case 0x32:
            return 'm';
        case 0x33:
            return ',';
        case 0x34:
            return '.';
        case 0x35:
            return '/';
        case 0x36:
            return 0x36; // Right Shift
        case 0x37:
            return '*';
        case 0x38:
            return 0x38; // Left Alt
        case 0x39:
            return ' ';
        case 0x3A:
            return 0x3A; // Caps Lock
        case 0x3B:
            return 0x3B; // F1
        case 0x3C:
            return 0x3C; // F2
        default:
            return 0x00;
    }
}

/* ========================================================================= */
