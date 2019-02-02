[BITS 32]
global io_hlt
global io_cli
global io_sti
global io_stihlt
global io_in8
global io_in16
global io_in32
global io_out8
global io_out6
global io_out32

global io_store_eflags
global io_load_eflags

global load_gdtr
global load_idtr

global asm_inthandler21
global asm_inthandler27
global asm_inthandler2c

extern inthandler21
extern inthandler27
extern inthandler2c

[SECTION .text]
; void io_hlt(void)
io_hlt:
    hlt
    ret

; void io_cli(void)
; disallow interrupt
io_cli:
    cli
    ret

; void io_sti(void)
; allow interrupt
io_sti:
    sti
    ret

; int io_in8(int port)
io_in8:
    mov edx, [esp+4]
    mov eax, 0
    in al, dx
    ret

; int in_in16(int port)
io_in16:
    mov edx, [esp+4]
    mov eax, 0
    in ax, dx
    ret

; int in_in16(int port)
io_in32:
    mov edx, [esp+4]
    in eax, dx
    ret

; void io_out8(int port, int data)
io_out8:
    mov edx, [esp+4]
    mov eax, [esp+8]
    out dx, al
    ret

; void io_out16(int port, int data)
io_out16:
    mov edx, [esp+4]
    mov eax, [esp+8]
    out dx, ax
    ret

; void io_out32(int port, int data)
io_out32:
    mov edx, [esp+4]
    mov eax, [esp+8]
    out dx, eax
    ret

; int io_load_eflags(void)
io_load_eflags:
    pushfd ; push eflags
    pop eax
    ret

; void io_store_eflags(int)
io_store_eflags:
    mov eax, [esp+4]
    push eax
    popfd ; pop eflags
    ret

; void load_gdtr(int limit, int addr)
load_gdtr:
    mov ax, [esp+4]
    mov [esp+6], ax
    lgdt [esp+6]
    ret

; void load_idtr(int limit, int addr)
load_idtr:
    mov ax, [esp+4]
    mov [esp+6], ax
    lidt [esp+6]
    ret

asm_inthandler21:
    push es
    push ds
    ; pushad pushes
    ; EAX ECX EDX EBX ESP EBP ESI EDI
    pushad
    mov eax, esp
    push eax
    mov ax, ss
    mov ds, ax
    mov es, ax
    call inthandler21
    pop eax
    popad
    pop ds
    pop es
    iretd

asm_inthandler27:
    push es
    push ds
    ; pushad pushes
    ; EAX ECX EDX EBX ESP EBP ESI EDI
    pushad
    mov eax, esp
    push eax
    mov ax, ss
    mov ds, ax
    mov es, ax
    call inthandler27
    pop eax
    popad
    pop ds
    pop es
    iretd

asm_inthandler2c:
    push es
    push ds
    ; pushad pushes
    ; EAX ECX EDX EBX ESP EBP ESI EDI
    pushad
    mov eax, esp
    push eax
    mov ax, ss
    mov ds, ax
    mov es, ax
    call inthandler2c
    pop eax
    popad
    pop ds
    pop es
    iretd