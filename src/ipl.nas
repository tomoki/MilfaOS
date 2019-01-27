; milfa-os
; TAB=4
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
    nop

    ; End of boot sector
    resb 448 - ($-$$-62) ; padding rest of BS_BootCode.
                         ; $ is current position, $$ is where this section starts.
                         ; In this nas, $$ is what org specifies (0x7c00).
                         ; Header is 64 byte, therefore $$-$-64 = BS_BootCode's actual size.
                         ; BS_BootCode must be 448 byte, so we need 448 - ($$-$-64) paddings.
                         ; https://www.nasm.us/xdoc/2.14.02/html/nasmdoc3.html#section-3.5
    db 0x55, 0xAA        ; BS_BootSign (2 byte)
