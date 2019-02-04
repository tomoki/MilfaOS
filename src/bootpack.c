#include <nasmfunc.h>
#include <bootpack.h>
#include <stdio.h>

void wait_keyboard_controller_ready(void)
{
    while (io_in8(PORT_KEYSTATUS) & KEYSTATUS_SEND_NOTREADY) ;
}

void init_keyboard(void)
{
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYCOMMAND, KEYCOMMAND_WRITE_MODE);
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYDATA, KEYBOARD_MODE);
    return;
}

void enable_mouse(void)
{
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYCOMMAND, KEYCOMMAND_SENDTO_MOUSE);
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYDATA, MOUSECOMMAND_ENABLE);
}

void MilfaMain(void)
{
    init_gdtidt();
    init_pic();
    init_keyboard();
    enable_mouse();

    unsigned char keybuf[32];
    initialize_ringbuffer_char(&keyboard_inputs, keybuf, 32);
    unsigned char mousebuf[128];
    initialize_ringbuffer_char(&mouse_inputs, mousebuf, 128);

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
        io_cli();
        int mouse_count = count_ringbuffer_char(&mouse_inputs);
        int keyboard_count = count_ringbuffer_char(&keyboard_inputs);
        if (keyboard_count > 0) {
            unsigned char data;
            get_ringbuffer_char(&keyboard_inputs, &data);
            char str[256];
            sprintf(str, "keyboard %d", data);
            box_fill(bootInfo->vram, bootInfo->screenWidth, 3, 0, 0, bootInfo->screenWidth, 15);
            putfont8_str(bootInfo->vram, bootInfo->screenWidth, str, font, 5, 0, 0);
        } else if (mouse_count > 0) {
            unsigned char data;
            get_ringbuffer_char(&mouse_inputs, &data);
            char str[256];
            sprintf(str, "mouse %d", data);
            box_fill(bootInfo->vram, bootInfo->screenWidth, 3, 0, 15, bootInfo->screenWidth, 30);
            putfont8_str(bootInfo->vram, bootInfo->screenWidth, str, font, 5, 0, 15);
        } else {
            io_sti();
            io_hlt();
        }
    }
}
