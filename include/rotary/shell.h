/* ========================================================================= */
/* Kernel Debugging Shell                                                    */
/* ========================================================================= */
#pragma once

#include <rotary/types.h>
#include <rotary/keyboard.h>
#include <rotary/vga.h>
#include <rotary/task.h>
#include <rotary/vmm.h>
#include <rotary/pmm.h>

/* ========================================================================= */

void shell_init();
void shell_keyboard_handler(uchar key);
void shell_print_prompt();
void shell_register_handler(char * command, void * handler);
void shell_process_command(char * command);

/* ========================================================================= */
