void io_hlt(void);
void io_cli(void);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void io_out8(int port, int data);

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

void MilfaMain(void)
{
    init_palette();
    // 0xa0000 ~ 0xaffff is framebuffer
    for (int i = 0xa0000; i <= 0xaffff; i++) {
        * ((char*)i) = i & 0x0f;
    }

fin:
    io_hlt();
    goto fin;
}