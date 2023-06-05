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

task_switch:
    ; EAX = current task
    mov     eax, [esp + 4]
    ; EDX = next task
    mov     edx, [esp + 8]

    ; Store registers that aren't preserved by cdecl
    push    ebp
    push    ebx
    push    esi
    push    edi

    ; We need to save the stack pointer of the task we're
    ; switching away from
    mov     [eax + 4], esp

    ; Now we need to load the stack pointer of the next task
    mov     esp, [edx + 4]

    ; Restore registers that aren't preserved by cdecl
    pop     edi
    pop     esi
    pop     ebx
    pop     ebp

    ; As we've changed ESP to that of the other task's stack,
    ; the return address on the stack that will be popped by
    ; this ret will take us into that task's code instead.
    ret
