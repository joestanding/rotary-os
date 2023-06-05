/* ========================================================================= */
/* Programmable Interrupt Timer                                              */
/* ========================================================================= */
#pragma once

#include <rotary/types.h>
#include <rotary/interrupts.h>
#include <rotary/io_port.h>
#include <rotary/task.h>
#include <rotary/logging.h>
#include <rotary/debug.h>

/* ========================================================================= */

uint32 timer_init();
uint32 timer_tick();

/* ========================================================================= */



