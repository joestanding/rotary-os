/* ========================================================================= */
/* Kernel Logging                                                            */
/* ========================================================================= */
#pragma once

#include <rotary/types.h>
#include <rotary/vga.h>
#include <rotary/serial.h>

/* ========================================================================= */

#define LOG_DEBUG 0

#define printk(log_level, fmt_str, ...) \
    { \
        char tmp[512]; \
        memset(&tmp, 0x00, 512); \
        sprintf(tmp, fmt_str, ##__VA_ARGS__); \
        _printk(log_level, LOG_TAG, tmp); \
    }

/* ========================================================================= */

void _printk(uint8 level, char * tag, char * message);

/* ========================================================================= */

