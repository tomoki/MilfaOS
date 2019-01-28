/* See https://vanya.jp.net/os/haribote.html#hrb */

OUTPUT_FORMAT("binary");

SECTIONS
{
    .head 0x00 : {
        LONG(64 * 1024)       /* 0-4 : Size of stack, .data, and heap */
        LONG(0x666c694d)      /* 4-8 : "Milf"                         */
        LONG(0)               /* 8-12: mmarea (?)                     */
        LONG(0x310000)        /* .data                                */
        LONG(SIZEOF(.data))
        LONG(LOADADDR(.data))
        LONG(0xE9000000)       /* 24-28 : 0xE9000000 (?)  */
        LONG(MilfaMain - 0x20) /* 28-32: Entry - 0x20 (?) */
        LONG(0)                /* 32-36: heap addr        */
    }

    .text : {*(.text)}
    .data 0x310000 : AT (ADDR(.text) + SIZEOF(.text)) {
        *(.data)
        *(.rodata*)
        *(.bss)
    }

    /* ? */
    /DISCARD/ : { *(.eh_frame) }
}