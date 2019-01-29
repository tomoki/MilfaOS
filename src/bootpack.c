#include <nasmfunc.h>
#include <hankaku.h>

void set_pallete(int start, int end, unsigned char* rgb)
{
    int backup_eflags = io_load_eflags();
    io_cli(); // prevent interruption

    int k = 0;
    for (int i = start; i <= end; i++) {
        // Note that each color should be 6 bit.
        io_out8(0x03c8, i);
        io_out8(0x03c9, rgb[3 * k + 0] >> 2);
        io_out8(0x03c9, rgb[3 * k + 1] >> 2);
        io_out8(0x03c9, rgb[3 * k + 2] >> 2);
        k++;
    }

    io_store_eflags(backup_eflags);
}

void init_palette(void)
{
    static unsigned char table_rgb[16 * 3] = {
        0x00, 0x00, 0x00, // 0: black
        0xff, 0x00, 0x00, // 1: red
        0x00, 0xff, 0x00, // 2: green
        0x00, 0x00, 0xff, // 3: blue
        0xff, 0xff, 0x00, // 4: yellow
        0xff, 0x00, 0xff, // 5: Purple
        0x00, 0xff, 0xff, // 6: light blur
        0xff, 0xff, 0xff, // 7: white
        0xc6, 0xc6, 0xc6, // 8: gray
        0xc6, 0x00, 0x00, // 9: dark red
        0x00, 0xc6, 0x00, // 10: dark green
        0x00, 0x00, 0xc6, // 11: dark blue
        0xc6, 0xc6, 0x00, // 12: dark yellow
        0xc6, 0x00, 0xc6, // 13: dark Purple
        0x00, 0xc6, 0xc6, // 14: dark light blur
        0xc6, 0xc6, 0xc6, // 15: dark white
    };

    set_pallete(0, 15, table_rgb);
}

void box_fill(unsigned char* vram, int width, unsigned char c, int x0, int y0, int x1, int y1)
{
    for (int x = x0; x <= x1; x++) {
        for (int y = y0; y <= y1; y++) {
            vram[y * width + x] = c;
        }
    }
    return;
}

void putfont8(unsigned char* vram, int width, char chara, unsigned char* font, unsigned char color, int x, int y)
{
    int index = chara * 16;
    for (int dy = 0; dy < 16; dy++) {
        int wy = y + dy;
        for (int dx = 0; dx < 8; dx++) {
            if (font[index + dy] & (1 << (7-dx)))
                vram[wy * width + x + dx] = color;
        }
    }
}

void putfont8_str(unsigned char* vram, int width, char* str, unsigned char* font, unsigned char color, int x, int y)
{
    while(*str != '\0') {
        putfont8(vram, width, *str, font, color, x, y);
        str++;
        x += 8;
    }
}

struct BootInfo {
    char cyls;
    char leds;
    char videoMode;
    char reserved0;
    short screenWidth;
    short screenHeight;
    unsigned char* vram;
};

void MilfaMain(void)
{
    init_palette();
    // 0xa0000 ~ 0xaffff is framebuffer
    for (int i = 0xa0000; i <= 0xaffff; i++) {
        * ((char*)i) = 14;
    }

    struct BootInfo* bootInfo = (struct BootInfo*)0x0ff0;

    box_fill(bootInfo->vram, bootInfo->screenWidth, 1, 20, 20, 120, 120);
    box_fill(bootInfo->vram, bootInfo->screenWidth, 2, 70, 50, 170, 150);
    box_fill(bootInfo->vram, bootInfo->screenWidth, 3, 120, 80, 220, 180);

    putfont8_str(bootInfo->vram, bootInfo->screenWidth, "MilfaOS", font, 0, 100, 100);

fin:
    io_hlt();
    goto fin;
}