
BOOTPACK_LOAD_TO EQU 0x00280000
DISK_CACHE       EQU 0x00100000
DISK_CACHE_REAL  EQU 0x00008000

; BOOT_INFO

CYLINDERS     EQU 0xff0
KEYBOARD_LEDS EQU 0xff1
SCREEN_VMODE  EQU 0xff2
SCREEN_WIDTH  EQU 0xff4
SCREEN_HEIGHT EQU 0xff6
SCREEN_VRAM   EQU 0xff8

    org 0xc200

; setup screen
    mov al, 0x13 ; 0x13 is 320x200x8bit color
    mov ah, 0x00
    int 0x10

; To refer these value from C as BootInfo, write them to memory
    mov byte [SCREEN_VMODE], 8
    mov word [SCREEN_WIDTH], 320
    mov word [SCREEN_HEIGHT], 200
    mov dword [SCREEN_VRAM], 0x000a0000

; Read keyboard shift/capslock status
    mov ah, 0x02
    int 0x16
    mov [KEYBOARD_LEDS], al

; Stop interruption from PIC
; This is needed before PIC initialization
    mov al, 0xff
    out 0x21, al ; 0x21 = PIC0_IMR
    nop          ; nop between outs is needed for some devices
    out 0xa1, al ; 0xa1 = PIC1_IMR
    cli

;  Setup A20Gate to allow CPU access 1MB> memory

    call waitkbdout
    mov  al, 0xd1
    out  0x64, al    ; 0x64 = PORT_KEYCOMMAND
    call waitkbdout
    mov  al, 0xdf    ; 0xdf = comamdn to enable A20 gate
    out  0x60, al    ; 0x60 = PORT_KEYDATA
    call waitkbdout ; wait command is processed

; Protected mode without paging
    lgdt [gdtr0] ; temporal gdt
    mov eax, cr0 ; https://wiki.osdev.org/CPU_Registers_x86
    and eax, 0x7fffffff ; make 31th bit 0 to disable paging
    or  eax,  0x00000001 ; make 0th bit 1 to enable protected mode
    mov cr0, eax
    jmp pipelineflush ; jmp is needed after chaning control register (CR0)

pipelineflush:
    mov ax, 1*8 ; 1*8 is second entry of gdt
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

; Transfer bootpack
    mov esi, bootpack
    mov edi, BOOTPACK_LOAD_TO
    mov ecx, 512*1024/4
    call memcpy

; Transfer diskdata
; boot sector
    mov esi, 0x7c00
    mov edi, DISK_CACHE
    mov ecx, 512/4
    call memcpy

; not boot sector
    mov esi, DISK_CACHE_REAL+512
    mov edi, DISK_CACHE+512
    mov ecx, 0
    mov cl, byte [CYLINDERS]
    imul ecx, 512*18*2/4
    sub ecx, 512/4 ; we don't have to copy ipl here
    call memcpy

; launch bootpack
    mov ebx, BOOTPACK_LOAD_TO
    mov ecx, [ebx+16]
    add ecx, 3 ; ecx += 3
    shr ecx, 2 ; ecx /= 4
    jz skip    ; nothing to transfer
    mov esi, [ebx+20]
    add esi, ebx
    mov edi, [ebx+12]
    call memcpy

skip:
    mov esp, [ebx+12]
    jmp dword 2*8:0x00000001b

waitkbdout:
    in al, 0x64
    and al, 0x02
    jnz waitkbdout
    ret

; memcpy copies each 1 byte
memcpy:
    mov eax, [esi]
    add esi, 4
    mov [edi], eax
    add edi, 4
    sub ecx, 1
    jnz memcpy
    ret

; Initial gdt
gdt0:
    resb 8
    dw 0xffff, 0x0000, 0x9200, 0x00cf
    dw 0xffff, 0x0000, 0x9a28, 0x0047
    dw 0

gdtr0:
    dw 8*3-1
    dd gdt0
    alignb 8

bootpack:
    ; In Makefile, we concatinate the below
