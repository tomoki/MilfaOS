#include <nasmfunc.h>
#include <bootpack.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

void task4_main(void)
{
    struct RingBufferChar timeout_buffer;
    unsigned char internal_buffer[100];
    initialize_ringbuffer_char(&timeout_buffer, internal_buffer, 100);

    set_timeout(&timeout_buffer, 5, 5000);

    while (1) {
        io_cli();
        int timer_count = count_ringbuffer_char(&timeout_buffer);
        if (timer_count == 0) {
            io_sti();
            io_hlt();
        } else {
            unsigned char data;
            get_ringbuffer_char(&timeout_buffer, &data);
            io_sti();
            if (data == 5) {
                taskswitch3();
                set_timeout(&timeout_buffer, 5, 5000);
            }
        }
    }
}

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

    init_pit();
    // Allow PIC1, keyboard, PIT
    // 11111001
    io_out8(PIC0_IMR, 0xf8);
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

    struct RingBufferChar timeout_buffer;
    initialize_ringbuffer_char(&timeout_buffer, malloc(sizeof(unsigned char) * 100), 100);

    int fired_per_1_sec = 0;
    int fired_per_3_sec = 0;
    int fired_per_10_sec = 0;

    set_timeout(&timeout_buffer, 1, 1000);
    set_timeout(&timeout_buffer, 3, 3000);
    set_timeout(&timeout_buffer, 10, 10000);

    struct TaskStatusSegment tss[2];
    tss[0].ldtr = 0;
    tss[0].iomap = 0x40000000;
    tss[1].ldtr = 0;
    tss[1].iomap = 0x40000000;

    // 103 == sizeof(struct SegmentDescriptor)?
    set_segment_descriptor(&((struct SegmentDescriptor*) ADDR_GDT)[3], 103, (int) (&tss[0]), AR_TSS32);
    set_segment_descriptor(&((struct SegmentDescriptor*) ADDR_GDT)[4], 103, (int) (&tss[1]), AR_TSS32);

    void* task4_stack = malloc(64 * 1024);
    tss[1].eip = (int)&task4_main;
    tss[1].eflags = 0x00000202; // IF = 1, default value?
    tss[1].eax = 0;
    tss[1].ecx = 0;
    tss[1].edx = 0;
    tss[1].ebx = 0;
    // stack grows to 0.
    tss[1].esp = (int)task4_stack + 64*1024;
    tss[1].ebp = 0;
    tss[1].edi = 0;
    // except cs, use asmhead's segment temporary.
    tss[1].es = 1 * 8;
    tss[1].cs = 2 * 8; // bootpack's code segment
    tss[1].ss = 1 * 8;
    tss[1].ds = 1 * 8;
    tss[1].fs = 1 * 8;
    tss[1].gs = 1 * 8;

    // Current program is 3rd of GDT.
    load_tr(3 * 8);

    while (1) {

        char str[256];
        sprintf(str, "%d cnt/sec, %d cnt/3sec, %d cnt/10sec", fired_per_1_sec, fired_per_3_sec, fired_per_10_sec);
        memset(keyboardInfoLayer->buffer, TRANSPARENT, keyboardInfoLayer->width * keyboardInfoLayer->height);
        putfont8_str(keyboardInfoLayer->buffer, keyboardInfoLayer->width, str, font, 5, 0, 0);
        layer_refresh_entire(layerControl, keyboardInfoLayer);

        io_cli();

        int mouse_count = count_ringbuffer_char(&mouse_inputs);
        int keyboard_count = count_ringbuffer_char(&keyboard_inputs);
        int timer_count = count_ringbuffer_char(&timeout_buffer);
        if (keyboard_count > 0) {
            io_sti();
            unsigned char data;
            get_ringbuffer_char(&keyboard_inputs, &data);
            char str[256];
            sprintf(str, "keyboard %d", data);
            memset(keyboardInfoLayer->buffer, TRANSPARENT, keyboardInfoLayer->width * keyboardInfoLayer->height);
            putfont8_str(keyboardInfoLayer->buffer, keyboardInfoLayer->width, str, font, 5, 0, 0);
            layer_refresh_entire(layerControl, keyboardInfoLayer);
        } else if (mouse_count > 0) {
            io_sti();
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
        } else if (timer_count > 0) {
            io_sti();
            unsigned char data;
            get_ringbuffer_char(&timeout_buffer, &data);

            if (data == 1) {
                fired_per_1_sec++;
                set_timeout(&timeout_buffer, 1, 1000);
            } else if(data == 3) {
                fired_per_3_sec++;
                set_timeout(&timeout_buffer, 3, 3000);
            } else if(data == 10) {
                // test
                taskswitch4();

                fired_per_10_sec++;
                set_timeout(&timeout_buffer, 10, 10000);

            }
        } else {
            io_sti();
            io_hlt();
        }
        layer_flush(layerControl);
    }
}
