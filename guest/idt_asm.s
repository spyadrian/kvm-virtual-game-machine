section .text                      ; start of the text (code) section
align 4                            ; the code must be 4 byte aligned

global idt_load

; Load the IDT
idt_load:
    mov     eax,[esp+4]  ; Get the pointer to the IDT, passed as a parameter. 
    lidt    [eax]        ; Load the IDT pointer.
    ret

; Low-level hardware interrupts handler for irq0
global _irq0
_irq0:
    push    0  ; put irq number on the stack
    jmp     irq_handler_wrapper

extern irq_handler

irq_handler_wrapper:
    ; Save all registers
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    esi
    push    edi
    push    ebp
    push    ds
    push    es
    push    fs
    push    gs

    ; Load kernel data descriptor into all segments
    mov     ax,0x10   ; 0x10 = kernel data segment selector (3rd selector)
    mov     ds,ax
    mov     es,ax
    mov     fs,ax
    mov     gs,ax

    ; What's on the stack at this point, in order (4 bytes each):
    ; gs, fs, es, ds, ebp, edi, esi, edx, ecx, ebx, eax, irq_number, eip, cs, eflags, esp, ss
    ; Pass irq_number as argument to the C function
    mov     eax,esp
    add     eax,44  ; to point to irq_number that was pushed in _irqX
    push    dword[eax]
    call    irq_handler
    pop     eax  ; only here to balance the "push eax" done before the call

    ; Restore all registers
    pop     gs
    pop     fs
    pop     es
    pop     ds
    pop     ebp
    pop     edi
    pop     esi
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
    
	; Fix-up the stack pointer due to the push done before the jump to irq_handler_wrapper
    add     esp,4
    iret
