#include <rotary/vga.h>

/* ========================================================================= */
/* Stuff                                                                     */
/* ========================================================================= */

struct s_vga_state {
    uint32 cursor_x;
    uint32 cursor_y;
    uint8  enabled;
} vga_state = {0, 3, 1};

void vga_set_cursor(uint32 x, uint32 y) {
    vga_state.cursor_x = x;
    vga_state.cursor_y = y;

    uint16 position = y * VGA_WIDTH + x;
    io_port_out(0x3D4, 0x0F);
    io_port_out(0x3D5, (unsigned char)(position & 0xFF));
    io_port_out(0x3D4, 0x0E);
    io_port_out(0x3D5, (unsigned char )((position >> 8) & 0xFF));
}

// ------------------------------------------------------------------------- //

void vga_print_char_at_cursor(uchar tchar) {
    vga_set_character(tchar, VGA_COLOUR_WHITE, VGA_COLOUR_BLACK, vga_state.cursor_x, vga_state.cursor_y);
    uint16 new_x = vga_state.cursor_x + 1;
    uint16 new_y = vga_state.cursor_y;

    if(vga_state.cursor_x == VGA_WIDTH) {
        vga_state.cursor_x = 0;
        new_y = vga_state.cursor_y + 1;
    }
    vga_set_cursor(new_x, new_y);
}

void vga_print_string_at_cursor(char * string) {
    for(uint32 i = 0; i < strlen(string); i++) {
        vga_print_char_at_cursor(string[i]);
    }
}

void vga_delete_char_at_cursor() {
    //TODO: what if we're already at 0
    if(vga_state.cursor_x == 0)
        return;
    vga_set_character(' ', VGA_COLOUR_BLACK, VGA_COLOUR_BLACK, vga_state.cursor_x - 1, vga_state.cursor_y);
    vga_state.cursor_x--;
}


// ------------------------------------------------------------------------- //

void vga_print_line(char * string) {
    vga_scroll_up(1, 4);
    vga_overwrite_line(string, VGA_COLOUR_WHITE, VGA_COLOUR_BLACK, 24);
}

// ------------------------------------------------------------------------- //

void vga_scroll_up(int line_count, int top_line) {
    //vga_clear_line(0); 
    for(uint32 i = 0; i < line_count; i++) {
        for(uint32 line = top_line; line < VGA_HEIGHT; line++) {
            for(uint32 x = 0; x < VGA_WIDTH; x++) {
                uint32 addr = VGA_BUFFER + (line * 160 + (x*2));
                uchar character = *(uchar*)addr;
                //TODO: Pull existing colour attributes
                vga_set_character(character, VGA_COLOUR_WHITE, VGA_COLOUR_BLACK, x, line-1);
            }
        }
    }
}

// ------------------------------------------------------------------------- //

void vga_clear_line(int line) {
    for(uint32 i = 0; i < 80; i++) {
        vga_set_character(' ', VGA_COLOUR_BLACK, VGA_COLOUR_BLACK, i, line);
    }
}

// ------------------------------------------------------------------------- //

void vga_clear_screen(uint32 colour) {
    for(uint32 x = 0; x < 80; x++) {
        for(uint32 y = 0; y < 25; y++) {
            vga_set_character(' ', VGA_COLOUR_BLACK, colour, x, y);
        }
    }
}

// ------------------------------------------------------------------------- //

void vga_overwrite_line(
        char * string,
        uchar fg_colour,
        uchar bg_colour,
        int y) {
    int string_len = strlen(string);
    for(uint32 i = 0; i < VGA_WIDTH; i++)
        vga_set_character(' ', VGA_COLOUR_BLACK, VGA_COLOUR_BLACK, i, y);
    for(uint32 i = 0; i < string_len; i++)
        vga_set_character(string[i], fg_colour, bg_colour, i, y);
}

// ------------------------------------------------------------------------- //

void vga_print_separator(uchar colour, int y) {
    for(uint32 i = 0; i < 80; i++) {
        vga_set_character(' ', VGA_COLOUR_BLACK, colour, i, y);
    }
}

// ------------------------------------------------------------------------- //

void vga_set_character(
        uchar character,
        uchar fg_colour, 
        uchar bg_colour,
        uint32 x,
        uint32 y) {
    uint16 attrib = (bg_colour << 4) | (fg_colour & 0x0F);
    volatile uint16 * video_memory;
    video_memory = (volatile uint16*)VGA_BUFFER + (y * 80 + x);
    *video_memory = character | (attrib << 8);
}

// ------------------------------------------------------------------------- //
