; milfa-os
; TAB=4

cyls equ 10 ; number of cylinders to read

    org 0x7c00  ; Where this program is loaded

; General FAT12 floppy configuration
; Refer: http://elm-chan.org/docs/fat.html

    jmp short entry  ; BS_JmpBoot     (3 byte)
    nop              ; "jmp short entry" generates 2 byte, so insert nop (0x90)
    db "milfa-os"    ; BS_OEMName     (8 byte)
    dw 512           ; BPB_BytsPerSec (2 byte)
    db 1             ; BPB_SecPerClus (1 byte)
    dw 1             ; BPB_RsvdSecCnt (2 byte)
    db 2             ; BPB_NumFATs    (1 byte)
    dw 224           ; BPB_RootEntCnt (2 byte)
    dw 2880          ; BPB_TotSec16   (2 byte), Drive's size, must be 2880
    db 0xf0          ; BPB_Media      (1 byte), media type, must be 0xf0
    dw 9             ; BPB_FATSz16    (2 byte)
    dw 18            ; BPB_SecPerTrk  (2 byte)
    dw 2             ; BPB_NumHeads   (2 byte)
    dd 0             ; BPB_HiddSec    (4 byte)
    dd 0             ; BPB_TotSec32   (4 byte), BPB_TotSec16 should be true value as it's less than 0x10000.
    db 0             ; BS_DrvNum      (1 byte)
    db 0             ; BS_Reserved1   (1 byte)
    db 0x29          ; BS_BootSig     (1 byte)
    dd 0x19921020    ; BS_VolID       (4 byte)
    db "MilfaOSVolu" ; BS_VolLab      (11 byte)
    db "FAT12   "    ; BS_FilSysType  (8 byte)

    ; BS_BootCode (448)
entry:
    mov ax, 0
    mov ss, ax
    mov sp, 0x7c00      ; program starts at 0x7c00, stack should grow from 0x7c00
    mov ds, ax

; Read disk
    mov ax, 0x0820        ; Read to 0x0820
    mov es, ax
    mov ch, 0            ; Cylinder 0
    mov dh, 0            ; Head 0
    mov cl, 2            ; Sector 2

readloop:
    mov ah, 0x02         ; Read disk
    mov al, 1            ; 1 Sector
    mov bx, 0            ; Will copy to [ES:BX]
    mov dl, 0x0          ; Drive 0 (?)
    int 0x13             ; Issue BIOS call
    jnc next             ; If carry flag is not set (= no error occurs), then go next
    ; Error
    jmp error
next:
    mov ax, es        ; What we want to do is "add es, 0x0020"
    add ax, 0x0020    ; but "es" register doesn't support that.
    mov es, ax        ; so we use "ax" temporary.
    add cl, 1         ; Add 1 to sector variable
    cmp cl, 18
    jbe readloop      ; if cl <= 18, then goto readloop
    mov cl, 1         ; Try next head.
    add dh, 1
    cmp dh, 2
    jb readloop       ; if dh (head) < 2, then goto readloop
    mov dh, 0
    add ch, 1
    cmp ch, cyls
    jb readloop       ; goto next cylinder

; Finish reading floppy.
; Execute our program
    mov [0x0ff0], ch  ; write how many cylinders we read to [0x0ff0]
    jmp 0xc200        ; program should be loaded to 0xc200

error:
    mov si, errormsg
putloop:
    mov al, [si]
    add si, 1
    cmp al, 0    ; check if it's null terminator
    je  fin      ; if null, then finish
    mov ah, 0x0e ; show 1 character
    mov bx, 15   ; color code
    int 0x10
    jmp putloop
fin:
    hlt
    jmp fin
errormsg:
    ; "\n\nloaderror\n"
    db 0x0a, 0x0a
    db "load error"
    db 0x0a
    db 0x00

    ; End of boot sector
    resb 448 - ($-$$-62) ; padding rest of BS_BootCode.
                         ; $ is current position, $$ is where this section starts.
                         ; In this nas, $$ is what org specifies (0x7c00).
                         ; Header is 62 byte, therefore $$-$-62 = BS_BootCode's actual size.
                         ; BS_BootCode must be 448 byte, so we need 448 - ($$-$-62) paddings.
                         ; https://www.nasm.us/xdoc/2.14.02/html/nasmdoc3.html#section-3.5
    db 0x55, 0xAA        ; BS_BootSign (2 byte)
