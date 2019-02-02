#include <bootpack.h>
#include <nasmfunc.h>

void init_pic(void)
{
    // --- interrupt off ---
    io_out8(PIC0_IMR, 0xff);
    io_out8(PIC1_IMR, 0xff);

    // ---- PIC0 setting ----
    // edge trigger mode
    io_out8(PIC0_ICW1, 0x11);
    // route IRQ 0-7 to INT 20-27
    io_out8(PIC0_ICW2, 0x20);
    // PIC1 is connected to IRQ 2
    io_out8(PIC0_ICW3, 1 << 2);
    // no buffer mode
    io_out8(PIC0_ICW4, 0x01);

    // ---- PIC1 setting ----
    // edge trigger mode
    io_out8(PIC1_ICW1, 0x11);
    // route IRQ 8-15 to INT 28-2f
    io_out8(PIC1_ICW2, 0x28);
    // PIC1 is connected to IRQ 2
    io_out8(PIC1_ICW3, 2);
    // no buffer mode
    io_out8(PIC1_ICW4, 0x01);

    // --- enable only PIC1 ---
    io_out8(PIC0_IMR, 0xfb); // 0xfb = 11111011
    io_out8(PIC1_IMR, 0xff);
}

// PS/2 keyboard
void inithandler21(int* esp)
{
    struct BootInfo* bootInfo = (struct BootInfo*) ADDR_BOOTINFO;
    box_fill(bootInfo->vram, bootInfo->screenWidth, 3, 0, 0, 32 * 8 - 1, 15);
    putfont8_str(bootInfo->vram, bootInfo->screenWidth, "INT 21 (IRQ-1): PS/2 keyboard", font, 5, 0, 0);
    while (1)
        io_hlt();
}

// PS/2 mouse
void inithandler2c(int* esp)
{
    struct BootInfo* bootInfo = (struct BootInfo*) ADDR_BOOTINFO;
    box_fill(bootInfo->vram, bootInfo->screenWidth, 3, 0, 0, 32 * 8 - 1, 15);
    putfont8_str(bootInfo->vram, bootInfo->screenWidth, "INT 2c (IRQ-1): PS/2 mouse", font, 5, 0, 0);
    while(1)
        io_hlt();
}
