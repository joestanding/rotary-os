global task_switch

; This routine switches tasks by saving registers that are not
; already preserved by the cdecl calling convention after this
; routine is called by task_schedule().

; The routine then stores the current stack pointer in the
; data structure for the current task, and updates ESP to the
; stored stack pointer in the structure for the next task.

; Once the stack pointer is updated, the following pops and
; ret will retrieve stored values from the next task's stack,
; including a return address that will transition EIP to the
; next task's code.


%define TASK_OFFSET_ID 0
%define TASK_OFFSET_KERNEL_ESP 4
%define TASK_OFFSET_KERNEL_EBP 8
%define TASK_OFFSET_KERNEL_STACK_FRAME 12
%define TASK_OFFSET_CR3 16
%define TASK_OFFSET_STATE 20
%define TASK_OFFSET_TICKS 24

task_switch:
    ; EAX = current task
    mov     eax, [esp + 4]
    ; EDX = next task
    mov     edx, [esp + 8]

    ; ------------------------------------------------------------- ;
    ; Storing Existing Task State
    ; --------------------------------------------------------------;

    ; Store registers that aren't preserved by cdecl
    push    ebp
    push    ebx
    push    esi
    push    edi

    ; Store this task's CR3 (PDE phys. addr)
    mov     ebx, cr3
    mov     [eax + TASK_OFFSET_CR3], ebx

    ; We need to save the stack pointer of the task we're
    ; switching away from (kernel_esp is second element of task struct)
    mov     [eax + TASK_OFFSET_KERNEL_ESP], esp

    ; ------------------------------------------------------------- ;
    ; Loading Next Task State
    ; --------------------------------------------------------------;

    ; Load the tasks's CR3
    mov     ebx, [edx + TASK_OFFSET_CR3]
    mov     cr3, ebx

    ; Now we need to load the stack pointer of the next task
    mov     esp, [edx + TASK_OFFSET_KERNEL_ESP]

    ; Restore registers that aren't preserved by cdecl
    pop     edi
    pop     esi
    pop     ebx
    pop     ebp

    ; As we've changed ESP to that of the other task's stack,
    ; the return address on the stack that will be popped by
    ; this ret will take us into that task's code instead.
    ret
