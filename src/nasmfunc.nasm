[BITS 32]
global io_hlt
global write_mem8

[SECTION .text]
; void io_hlt(void)
io_hlt:
    hlt
    ret

; void write_mem8(int addr, int data)
write_mem8:
    mov ecx, [esp+4]
    mov al, [esp+8]
    mov [ecx], al
    ret
