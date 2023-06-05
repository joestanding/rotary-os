/* ========================================================================= */
/* x86 Interrupts                                                            */
/* ========================================================================= */
#pragma once

/* ========================================================================= */

#include <rotary/types.h>
#include <rotary/debug.h>
#include <rotary/logging.h>
#include <rotary/task.h>
#include <rotary/pic8259.h>

#undef  LOG_TAG
#define LOG_TAG "interrupts"

#define low_16(address) (uint16)((address) & 0xFFFF)
#define high_16(address) (uint16)(((address) >> 16) & 0xFFFF)

#define MAX_INTERRUPT_HANDLERS 256

#define INT_DIV_BY_ZERO         0
#define INT_SINGLE_STEP         1
#define INT_NMI                 2
#define INT_BREAKPOINT          3
#define INT_OVERFLOW            4
#define INT_BOUND_EXCEEDED      5
#define INT_INVALID_OPCODE      6
#define INT_FPU_NOT_AVAIL       7
#define INT_DOUBLE_FAULT        8
#define INT_COPRO_SEG_ORUN      9
#define INT_INVALID_TSS         10
#define INT_SEG_NOT_PRESENT     11
#define INT_STACK_SEG_FAULT     12
#define INT_GENERAL_PROT_FAULT  13
#define INT_PAGE_FAULT          14
#define INT_RESERVED            15
#define INT_X87_FPU_EXCEPTION   16
#define INT_ALIGNMENT_CHECK     17
#define INT_MACHINE_CHECK       18
#define INT_SIMD_FPU_EXCEPTION  19
#define INT_VIRT_EXCEPTION      20
#define CONTROL_PROT_EXCEPTION  21

#define INT_PIT                 32
#define INT_KEYBOARD            33

/* ========================================================================= */

// Represents each IDT gate in the IDT table
typedef struct {
    uint16 low_offset;
    uint16 segment_selector;
    uint8  reserved;

    uint8  attributes;
    uint16 high_offset;
} __attribute__((packed)) idt_gate;

typedef struct {
    uint16 limit;
    uint32 base;
} __attribute__((packed)) idt_pointer;

// Contains information passed through to the generic ISR handler
// from the ISR assembly stub routine
typedef struct {
    uint32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32 int_num, error_code;
    uint32 eip, cs, eflags;
} isr_registers;

typedef void func(isr_registers * registers);

/* ========================================================================= */

void set_idt_gate(uint32 int_num, uint32 handle_addr);
uint32 enable_hardware_interrupts();
void disable_hardware_interrupts();
void interrupt_init();
void register_interrupt_handler(uint32 int_num, void * handler_addr);

// Assign our ISR handlers as exported from isr.asm to our IDT
uint32 idt_init();
// Load our populated IDT with LIDT
uint32 idt_load();
// Called by the generic ISR stub routine in isr.asm
void isr_handler(isr_registers * registers);

/* ========================================================================= */
/* Interrupt Service Routines                                                */
/* ========================================================================= */

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr32();
extern void isr33();
extern void isr34();
extern void isr35();
extern void isr36();
extern void isr37();
extern void isr38();
extern void isr39();
extern void isr40();
extern void isr41();
extern void isr42();
extern void isr43();
extern void isr44();
extern void isr45();
extern void isr46();
extern void isr47();

/* ========================================================================= */
