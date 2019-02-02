#include <nasmfunc.h>
#include <bootpack.h>
#include <stdio.h>

void MilfaMain(void)
{
    init_gdtidt();
    init_pic();
    io_sti();

    init_palette();
    struct BootInfo* bootInfo = (struct BootInfo*) ADDR_BOOTINFO;

    // box_fill(bootInfo->vram, bootInfo->screenWidth, 1, 20, 20, 120, 120);
    // box_fill(bootInfo->vram, bootInfo->screenWidth, 2, 70, 50, 170, 150);
    // box_fill(bootInfo->vram, bootInfo->screenWidth, 3, 120, 80, 220, 180);

    putfont8_str(bootInfo->vram, bootInfo->screenWidth, "MilfaOS", font, 0, 100, 100);
        // 0xa0000 ~ 0xaffff is framebuffer
    for (int i = 0xa0000; i <= 0xaffff; i++) {
        * ((char*)i) = 14;
    }

    char s[256];
    sprintf(s, "scrnx = %d, scrny = %d", bootInfo->screenWidth, bootInfo->screenHeight);
    putfont8_str(bootInfo->vram, bootInfo->screenWidth, s, font, 0, 100, 120);
    unsigned char mouse[16*16];
    init_mouse_cursor8(mouse, 14);
    put_block8(bootInfo->vram, bootInfo->screenWidth, mouse, 16, 16, 50, 50);

    // Allow PIC1 and keyboard
    // 11111001
    io_out8(PIC0_IMR, 0xf9);
    // Allow mouse
    // 11101111
    io_out8(PIC1_IMR, 0xef);

    while (1) {
        io_hlt();
    }
}