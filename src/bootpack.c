#include <nasmfunc.h>
#include <bootpack.h>
#include <stdio.h>
#include <stdlib.h>


void MilfaMain(void)
{
    init_gdtidt();
    init_pic();
    init_keyboard();

    struct MouseData mouse_data;
    enable_mouse(&mouse_data);

    unsigned char keybuf[32];
    initialize_ringbuffer_char(&keyboard_inputs, keybuf, 32);
    unsigned char mousebuf[128];
    initialize_ringbuffer_char(&mouse_inputs, mousebuf, 128);

    io_sti();

    // 0x00000000 ~ 0x004000000 is used in boot sequence
    size_t memory_size = memtest(0x00400000, 0xbfffffff);
    init_malloc((void*) 0x00400000, memory_size - 0x00400000);

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

    sprintf(s, "memory = %d MB  free: %d MB", memory_size / 1024 / 1024, malloc_free_size() / 1024 / 1024);
    putfont8_str(bootInfo->vram, bootInfo->screenWidth, s, font, 0, 100, 150);

    unsigned char mouse[16*16];
    init_mouse_cursor8(mouse, 14);
    int mouse_x = 16, mouse_y = 16;

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
            int mouse_data_ready = decode_mouse(&mouse_data, data);
            if (mouse_data_ready) {
                char str[256];
                sprintf(str, "mouse %d %d %d", mouse_data.button, mouse_data.x, mouse_data.y);
                box_fill(bootInfo->vram, bootInfo->screenWidth, 3, 0, 15, bootInfo->screenWidth, 30);
                putfont8_str(bootInfo->vram, bootInfo->screenWidth, str, font, 5, 0, 15);

                // Erase previous
                box_fill(bootInfo->vram, bootInfo->screenWidth, 14, mouse_x, mouse_y, mouse_x + 16, mouse_y + 16);
                mouse_x += mouse_data.x;
                mouse_y += mouse_data.y;
                // Draw mouse
                put_block8(bootInfo->vram, bootInfo->screenWidth, mouse, 16, 16, mouse_x, mouse_y);
            }
        } else {
            io_sti();
            io_hlt();
        }
    }
}
