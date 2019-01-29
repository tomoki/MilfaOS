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
global write_mem8

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