#include <nasmfunc.h>
#include <bootpack.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

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

    // Allow PIC1 and keyboard
    // 11111001
    io_out8(PIC0_IMR, 0xf9);
    // Allow mouse
    // 11101111
    io_out8(PIC1_IMR, 0xef);

    // 0x00000000 ~ 0x004000000 is used in boot sequence
    size_t memory_size = memtest(0x00400000, 0xbfffffff);
    init_malloc((void*) 0x00400000, memory_size - 0x00400000);

    init_palette();
    struct BootInfo* bootInfo = (struct BootInfo*) ADDR_BOOTINFO;

    struct LayerControl* layerControl = init_layer_control(bootInfo->vram, bootInfo->screenWidth, bootInfo->screenHeight);

    struct Layer* backgroundLayer = layer_create(layerControl, 0, 0, bootInfo->screenWidth, bootInfo->screenHeight);
    {
        memset(backgroundLayer->buffer, 14, backgroundLayer->width * backgroundLayer->height);
        layer_refresh_entire(layerControl, backgroundLayer);
        layer_change_zindex(layerControl, backgroundLayer, -100);
    }

    struct Layer* infoLayer = layer_create(layerControl, 0, 150, 300, 40);
    {
        char s[256];
        sprintf(s, "memory = %d MB  free: %d MB", memory_size / 1024 / 1024, malloc_free_size() / 1024 / 1024);
        putfont8_str(infoLayer->buffer, infoLayer->width, s, font, 0, 0, 0);

        sprintf(s, "scrnx = %d, scrny = %d", bootInfo->screenWidth, bootInfo->screenHeight);
        putfont8_str(infoLayer->buffer, infoLayer->width, s, font, 0, 0, 20);
        layer_refresh_entire(layerControl, infoLayer);
    }

    int mouse_x = 16, mouse_y = 16;
    struct Layer* mouseLayer = layer_create(layerControl, mouse_x, mouse_y, 16, 16);
    {
        init_mouse_cursor8(mouseLayer->buffer, TRANSPARENT);
        layer_refresh_entire(layerControl, mouseLayer);
        layer_change_zindex(layerControl, mouseLayer, 100);
    }

    struct Layer* keyboardInfoLayer = layer_create(layerControl, 0, 0, 300, 20);
    {
        layer_refresh_entire(layerControl, keyboardInfoLayer);
    }

    struct Layer* mouseInfoLayer = layer_create(layerControl, 0, 20, 300, 20);
    {
        layer_refresh_entire(layerControl, mouseInfoLayer);
    }

    layer_flush(layerControl);

    int counter = 0;
    while (1) {

        char str[256];
        sprintf(str, "counter %d", counter++);
        memset(keyboardInfoLayer->buffer, TRANSPARENT, keyboardInfoLayer->width * keyboardInfoLayer->height);
        putfont8_str(keyboardInfoLayer->buffer, keyboardInfoLayer->width, str, font, 5, 0, 0);
        layer_refresh_entire(layerControl, keyboardInfoLayer);

        io_cli();

        int mouse_count = count_ringbuffer_char(&mouse_inputs);
        int keyboard_count = count_ringbuffer_char(&keyboard_inputs);
        if (keyboard_count > 0) {
            unsigned char data;
            get_ringbuffer_char(&keyboard_inputs, &data);
            char str[256];
            sprintf(str, "keyboard %d", data);
            memset(keyboardInfoLayer->buffer, TRANSPARENT, keyboardInfoLayer->width * keyboardInfoLayer->height);
            putfont8_str(keyboardInfoLayer->buffer, keyboardInfoLayer->width, str, font, 5, 0, 0);
            layer_refresh_entire(layerControl, keyboardInfoLayer);
        } else if (mouse_count > 0) {
            unsigned char data;
            get_ringbuffer_char(&mouse_inputs, &data);
            int mouse_data_ready = decode_mouse(&mouse_data, data);
            if (mouse_data_ready) {
                mouse_x += mouse_data.x;
                mouse_y += mouse_data.y;
                mouse_x = MAX(0, MIN(bootInfo->screenWidth, mouse_x));
                mouse_y = MAX(0, MIN(bootInfo->screenHeight, mouse_y));
                // Draw mouse
                // put_block8(bootInfo->vram, bootInfo->screenWidth, mouse, 16, 16, mouse_x, mouse_y);
                layer_move(layerControl, mouseLayer, mouse_x, mouse_y);

                char str[256];
                sprintf(str, "mouse %d %d %d", mouse_data.button, mouseLayer->x, mouseLayer->y);
                memset(mouseInfoLayer->buffer, TRANSPARENT, mouseInfoLayer->width * mouseInfoLayer->height);
                putfont8_str(mouseInfoLayer->buffer, mouseInfoLayer->width, str, font, 5, 0, 0);
                layer_refresh_entire(layerControl, mouseInfoLayer);
            }
        } else {
        }
        io_sti();
        layer_flush(layerControl);
    }
}
