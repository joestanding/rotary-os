/* ========================================================================= */
/* PS/2 Keyboard Input                                                       */
/* ========================================================================= */
#include <rotary/types.h>
#include <rotary/interrupts.h>
#include <rotary/io_port.h>
#include <rotary/vga.h>

/* ========================================================================= */

#define KEY_ENTER       0x1C
#define KEY_BACKSPACE   0x0E

/* ========================================================================= */

typedef void keyboard_handler(uchar key);

/* ========================================================================= */

uint32 driver_keyboard_init();
void driver_keyboard_isr(isr_registers * registers);
char driver_keyboard_translate_scan_code(uint8 scan_code);
void driver_keyboard_register_handler(uint32 handler_addr);

/* ========================================================================= */
