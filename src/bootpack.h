#pragma once

// asmhead.nas

struct BootInfo {
    char cyls;
    char leds;
    char videoMode;
    char reserved0;
    short screenWidth;
    short screenHeight;
    unsigned char* vram;
};

#define ADDR_BOOTINFO 0x0ff0

// graphics.c

void set_pallete(int start, int end, unsigned char* rgb);
void init_mouse_cursor8(unsigned char* mouse, unsigned char background);
void init_palette(void);
void put_block8(unsigned char* vram, int width, unsigned char* block, int blockwidth, int blockheight, int x, int y);
void box_fill(unsigned char* vram, int width, unsigned char c, int x0, int y0, int x1, int y1);
void putfont8_str(unsigned char* vram, int width, char* str, unsigned char* font, unsigned char color, int x, int y);

// desciptors.c

#define ADDR_GDT 0x00270000
#define ADDR_IDT 0x0026f800
#define LIMIT_GDT 0x0000ffff
#define AR_DATA32_RW 0x4092
#define AR_CODE32_ER 0x409a
#define ADDR_BOOTPACK 0x00280000
#define LIMIT_BOOTPACK 0x0007ffff
#define LIMIT_IDT 0x000007ff
struct SegmentDescriptor {
    short limit_low;
    short base_low;
    char base_mid;
    char access_right;
    char limit_high;
    char base_high;
};

struct GateDescriptor {
    short offset_low;
    short selector;
    char dw_count;
    char access_right;
    short offset_high;
};

void set_segment_descriptor(struct SegmentDescriptor* sd, unsigned int limit, int base, int ar);
void set_gate_descriptor(struct GateDescriptor* gd, int offset, int selector, int ar);
void init_gdtidt(void);

// int.c
#define PIC0_IMR  0x0021
#define PIC0_ICW1 0x0020
#define PIC0_ICW2 0x0021
#define PIC0_ICW3 0x0021
#define PIC0_ICW4 0x0021
#define PIC0_OCW2 0x0020

#define PIC1_IMR   0x00a1
#define PIC1_ICW1  0x00a0
#define PIC1_ICW2  0x00a1
#define PIC1_ICW3  0x00a1
#define PIC1_ICW4  0x00a1
#define PIC1_OCW2  0x00a0
void init_pic(void);
void inithandler21(int* esp);
void inithandler2c(int* esp);

// hankaku.cpp
extern char font[4096];