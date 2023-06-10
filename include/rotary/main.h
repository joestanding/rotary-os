#pragma once

#include <rotary/types.h>
#include <rotary/multiboot.h>
#include <rotary/gdt.h>
#include <rotary/tss.h>
#include <rotary/vga.h>
#include <rotary/interrupts.h>
#include <rotary/keyboard.h>
#include <rotary/shell.h>
#include <rotary/pic8259.h>
#include <rotary/vmm.h>
#include <rotary/debug.h>
#include <rotary/logging.h>
#include <rotary/serial.h>
#include <rotary/timer.h>
#include <rotary/paging.h>
#include <rotary/pmm.h>
#include <rotary/test.h>

#undef  LOG_TAG
#define LOG_TAG "init"

#define OK_STR "[ \fOK\v ]\n"
#define FAIL_STR "[\rFAIL\v]\n"

