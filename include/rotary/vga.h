/* ========================================================================= */
/* VGA Output                                                                */
/* ========================================================================= */
#pragma once

#include <rotary/types.h>
#include <rotary/string.h>
#include <rotary/io_port.h>

/* ========================================================================= */

#define TOP_LINE    3

#define VGA_BUFFER  0xc00b8000
#define VGA_WIDTH   80
#define VGA_HEIGHT  25

#define VGA_COLOUR_BLACK        0
#define VGA_COLOUR_BLUE         1
#define VGA_COLOUR_GREEN        2
#define VGA_COLOUR_CYAN         3
#define VGA_COLOUR_RED          4
#define VGA_COLOUR_MAGENTA      5
#define VGA_COLOUR_BROWN        6
#define VGA_COLOUR_WHITE        7

#define vga_printf(fmt_str, ...) \
    { \
        char tmp[512]; \
        memset(&tmp, 0, sizeof(tmp)); \
        sprintf(tmp, fmt_str, ##__VA_ARGS__); \
        vga_write(tmp); \
    }

/* ========================================================================= */

typedef struct {
    uint32 cursor_x;
    uint32 cursor_y;
    uint8  enabled;
} vga_state_t;

extern vga_state_t vga_state;

/* ========================================================================= */

void vga_write(char * string);
void vga_set_cursor(uint32 x, uint32 y);
void vga_print_char_at_cursor(uchar tchar);
void vga_print_string_at_cursor(char * string);
void vga_delete_char_at_cursor();
void vga_print_line(char * string);
void vga_scroll_up(int line_count, int top_line);
void vga_clear_screen();
void vga_clear_line(int line);
void vga_overwrite_line(char * string, uchar fg_colour, uchar bg_colour, int y);
void vga_set_character(uchar character, uchar fg_colour, uchar bg_colour, uint32 x, uint32 y);
void vga_print_separator(uchar colour, int y);

/* ========================================================================= */
