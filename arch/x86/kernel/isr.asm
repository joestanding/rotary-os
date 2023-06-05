extern isr_handler

isr_dispatch:
    cli                     ; Disable interrupts while we handle this
    pushad                  ; Push all general purpose registers to the stack
    push esp
    call    isr_handler     ; Call the C handler
    pop eax
    popad                   ; Pop 
    add     esp, 8
    sti
    iret

global isr_exit
isr_exit:
    popad
    add     esp, 8 ; skip int_num, error_code
    sti
    iret

; Interrupt Service Routines (ISRs)
; These will be called by the CPU after being referenced from the Interrupt Descriptor Table (IDT)

global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31
global isr32
global isr33
global isr34
global isr35
global isr36
global isr37
global isr38
global isr39
global isr40
global isr41
global isr42
global isr43
global isr44
global isr45
global isr46
global isr47

; Divide by Zero Exception
; Error Code: No
isr0:
    push    byte 0
    push    byte 0
    jmp     isr_dispatch

; Debug
; Error Code: No
isr1:
    push    byte 0
    push    byte 1
    jmp    isr_dispatch

; Non-maskable Interrupt
; Error Code: No
isr2:
    push    byte 0
    push    byte 2
    jmp    isr_dispatch

; Breakpoint
; Error Code: No
isr3:
    push    byte 0
    push    byte 3
    jmp    isr_dispatch

; Overflow
; Error Code: No
isr4:
    push    byte 0
    push    byte 4
    jmp    isr_dispatch

; Bound Range Exceeded
; Error Code: No
isr5:
    push    byte 0
    push    byte 5
    jmp    isr_dispatch

; Invalid Opcode
; Error Code: No
isr6:
    push    byte 0
    push    byte 6
    jmp    isr_dispatch

; Device Not Available
; Error Code: No
isr7:
    push    byte 0
    push    byte 7
    jmp    isr_dispatch

; Double Fault
; Error Code: Yes
isr8:
    push    byte 8
    jmp    isr_dispatch

; Coprocessor Segment Overrun
; Error Code: No
isr9:
    push    byte 0
    push    byte 9
    jmp    isr_dispatch

; Invalid TSS
; Error Code: Yes
isr10:
    push    byte 10
    jmp    isr_dispatch

; Segment Not Present
; Error Code: Yes
isr11:
    push    byte 11
    jmp    isr_dispatch

; Stack-Segment Fault
; Error Code: Yes
isr12:
    push    byte 12
    jmp    isr_dispatch

; General Protection Fault
; Error Code: Yes
isr13:
    push    byte 13
    jmp    isr_dispatch

; Page Fault
; Error Code: Yes
isr14:
    push    byte 14
    jmp    isr_dispatch

; Reserved
; Error Code: No
isr15:
    push    byte 0
    push    byte 15
    jmp    isr_dispatch

; x87 Floating Point Exception
; Error Code: No
isr16:
    push    byte 0
    push    byte 16
    jmp    isr_dispatch

; Alignment Check
; Error Code: Yes
isr17:
    push    byte 17
    jmp     isr_dispatch

; Machine Check
; Error Code: No
isr18:
    push    byte 0
    push    byte 18
    jmp    isr_dispatch

; SIMD Floating Point Exception
; Error Code: No
isr19:
    push    byte 0
    push    byte 19
    jmp    isr_dispatch

; Virtualisation Exception
; Error Code: No
isr20:
    push    byte 0
    push    byte 20
    jmp    isr_dispatch

; Control Protection Exception
; Error Code: Yes
isr21:
    push    byte 21
    jmp    isr_dispatch

isr22:
    push    byte 0
    push    byte 22
    jmp    isr_dispatch

isr23:
    push    byte 0
    push    byte 23
    jmp    isr_dispatch

isr24:
    push    byte 0
    push    byte 24
    jmp    isr_dispatch

isr25:
    push    byte 0
    push    byte 25
    jmp    isr_dispatch

isr26:
    push    byte 0
    push    byte 26
    jmp    isr_dispatch

isr27:
    push    byte 0
    push    byte 27
    jmp    isr_dispatch

isr28:
    push    byte 0
    push    byte 28
    jmp    isr_dispatch

isr29:
    push    byte 0
    push    byte 29
    jmp    isr_dispatch

isr30:
    push    byte 0
    push    byte 30
    jmp    isr_dispatch

isr31:
    push    byte 0
    push    byte 31
    jmp    isr_dispatch

; =============================================== ;

isr32:
    push byte 0
    push byte 32
    jmp isr_dispatch

isr33:
    push byte 1
    push byte 33
    jmp isr_dispatch

isr34:
    push byte 2
    push byte 34
    jmp isr_dispatch

isr35:
    push byte 3
    push byte 35
    jmp isr_dispatch

isr36:
    push byte 4
    push byte 36
    jmp isr_dispatch

isr37:
    push byte 5
    push byte 37
    jmp isr_dispatch

isr38:
    push byte 6
    push byte 38
    jmp isr_dispatch

isr39:
    push byte 7
    push byte 39
    jmp isr_dispatch

isr40:
    push byte 8
    push byte 40
    jmp isr_dispatch

isr41:
    push byte 9
    push byte 41
    jmp isr_dispatch

isr42:
    push byte 10
    push byte 42
    jmp isr_dispatch

isr43:
    push byte 11
    push byte 43
    jmp isr_dispatch

isr44:
    push byte 12
    push byte 44
    jmp isr_dispatch

isr45:
    push byte 13
    push byte 45
    jmp isr_dispatch

isr46:
    push byte 14
    push byte 46
    jmp isr_dispatch

isr47:
    push byte 15
    push byte 47
    jmp isr_dispatch

