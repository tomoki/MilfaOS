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
#define MAX_NUMBER_OF_LAYERS 400
#define TRANSPARENT 35

struct Layer {
    unsigned char* buffer;
    int width;
    int height;
    int x;
    int y;
    int zindex;
};

struct Rect {
    int x;
    int y;
    int width;
    int height;
};

struct LayerControl {
    unsigned char* vram;
    int width;
    int height;
    int number_of_layers;
    struct Layer layers[MAX_NUMBER_OF_LAYERS];
    // sorted by z-index
    struct Layer* sorted_layers[MAX_NUMBER_OF_LAYERS];

    struct Rect dirty_rect;
};

// x0, y0, x1, y1 is local coordinate in layer
struct Layer* layer_create(struct LayerControl* lc, int x, int y, int width, int height);
void layer_refresh(struct LayerControl* lc, struct Layer* layer, int x0, int y0, int x1, int y1);
void layer_refresh_entire(struct LayerControl* lc, struct Layer* layer);
void layer_move(struct LayerControl* lc, struct Layer* layer, int x, int y);
void layer_change_zindex(struct LayerControl* lc, struct Layer* layer, int zindex);
struct LayerControl* init_layer_control(unsigned char* vram, int width, int height);
void layer_flush(struct LayerControl* lc);

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
#define AR_INTGATE32 0x008e
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

// void set_segment_descriptor(struct SegmentDescriptor* sd, unsigned int limit, int base, int ar);
// void set_gate_descriptor(struct GateDescriptor* gd, int offset, int selector, int ar);
// void init_gdtidt(void);

void init_gdtidt(void);
void set_segment_descriptor(struct SegmentDescriptor *sd, unsigned int limit, int base, int ar);
void set_gate_descriptor(struct GateDescriptor *gd, int offset, int selector, int ar);

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
// See http://oswiki.osask.jp/?cmd=read&page=%28AT%29keyboard&word=keyboard
#define PORT_KEYDATA 0x0060
#define PORT_KEYSTATUS 0x0064
#define PORT_KEYCOMMAND 0x0064
#define KEYCOMMAND_WRITE_MODE 0x60
#define KEYCOMMAND_SENDTO_MOUSE 0xd4
#define MOUSECOMMAND_ENABLE 0xf4
#define KEYSTATUS_SEND_NOTREADY 0x02
// XXX: why 0x47?
#define KEYBOARD_MODE 0x47

struct RingBufferChar {
    unsigned char *buffer;
    int size;
    int free;
    int rp, wp;
};

struct RingBufferChar keyboard_inputs;
struct RingBufferChar mouse_inputs;
void initialize_ringbuffer_char(struct RingBufferChar*, unsigned char* buf, int size);
int count_ringbuffer_char(struct RingBufferChar*);
int put_ringbuffer_char(struct RingBufferChar*, unsigned char data);
int get_ringbuffer_char(struct RingBufferChar*, unsigned char* data);

void init_pic(void);
void inthandler21(int* esp);
void inthandler27(int* esp);
void inthandler2c(int* esp);

// mouse.c
#define MOUSE_LEFT_CLICK (1 << 0)
#define MOUSE_RIGHT_CLICK (1 << 1)
#define MOUSE_CENTER_CLICK (1 << 2)

struct MouseData {
    // Metadata
    unsigned char buf[3];
    int phase;

    // Extracted data
    int x;
    int y;
    char button;
};

void wait_keyboard_controller_ready(void);
void init_keyboard(void);
void enable_mouse(struct MouseData*);
// return 1 if MouseData.buf is fullfilled.
int decode_mouse(struct MouseData*, unsigned int next_data);

// hankaku.cpp
extern unsigned char font[4096];
