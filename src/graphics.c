#include <bootpack.h>
#include <nasmfunc.h>
#include <stdlib.h>
#include <math.h>

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

void init_mouse_cursor8(unsigned char* mouse, unsigned char background)
{
    static char cursor[16][16] = {
        "xx..............",
        "xox.............",
        "xoox............",
        "xooox...........",
        "xoooox..........",
        "xooooox.........",
        "xoooooox........",
        "xooooooox.......",
        "xoooooooox......",
        "xooooooooox.....",
        "xoooooooooox....",
        "xxxxxooooox.....",
        "x....xxox.......",
        ".......xox......",
        "........xxx.....",
        "..........x....."
    };
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            if (cursor[y][x] == '.')
                mouse[y * 16 + x] = background;
            else if(cursor[y][x] == 'o')
                mouse[y * 16 + x] = 7; // white
            else if(cursor[y][x] == 'x')
                mouse[y * 16 + x] = 0; // black
        }
    }
}

void put_block8(unsigned char* vram, int width, unsigned char* block, int blockwidth, int blockheight, int x, int y)
{
    for (int dy = 0; dy < blockheight; dy++) {
        for (int dx = 0; dx < blockwidth; dx++) {
            vram[(y + dy) * width + (x + dx)] = block[dy * blockwidth + dx];
        }
    }
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

// Dirty rect
void merge_dirty_rect(struct Rect* merge_into, struct Rect* r)
{
    if (merge_into->width == 0 && merge_into->height == 0) {
        merge_into->x = r->x;
        merge_into->y = r->y;
        merge_into->width = r->width;
        merge_into->height = r->height;
    } else {
        int x0 = MIN(merge_into->x, r->x);
        int x1 = MAX(merge_into->x + merge_into->width, r->x + r->width);
        int y0 = MIN(merge_into->y, r->y);
        int y1 = MAX(merge_into->y + merge_into->height, r->y + r->height);

        merge_into->x = x0;
        merge_into->y = y0;
        merge_into->width = x1 - x0;
        merge_into->height = y1 - y0;
    }
}

void merge_dirty_layer_rect(struct Rect* merge_into, struct Layer* l)
{
    struct Rect rect;
    rect.x = l->x;
    rect.y = l->y;
    rect.width = l->width;
    rect.height = l->height;
    merge_dirty_rect(merge_into, &rect);
}

void layer_global_refresh(struct LayerControl* lc, int gx0, int gx1, int gy0, int gy1)
{
    gx0 = MIN(MAX(0, gx0), lc->width);
    gx1 = MIN(MAX(0, gx1), lc->width);
    gy0 = MIN(MAX(0, gy0), lc->height);
    gy1 = MIN(MAX(0, gy1), lc->height);

    for (int x = gx0; x < gx1; x++) {
        for (int y = gy0; y < gy1; y++) {
            for (int i = lc->number_of_layers-1; i >= 0; i--) {
                struct Layer* t = lc->sorted_layers[i];
                if (t->x <= x && x < t->x + t->width &&
                    t->y <= y && y < t->y + t->height) {
                    int lx = x - t->x;
                    int ly = y - t->y;
                    lc->vram[y * lc->width + x] = t->buffer[ly * t->width + lx];
                    break;
                }
            }
        }
    }
}

// x0, y0, x1, y1 is local coordinate in layer
void layer_refresh(struct LayerControl* lc, struct Layer* layer, int x0, int y0, int x1, int y1)
{
    merge_dirty_layer_rect(&lc->dirty_rect, layer);
}

void layer_refresh_entire(struct LayerControl* lc, struct Layer* layer)
{
    layer_refresh(lc, layer, 0, 0, layer->width, layer->height);
}

void layer_move(struct LayerControl* lc, struct Layer* layer, int x, int y)
{
    merge_dirty_layer_rect(&lc->dirty_rect, layer);
    layer->x = x;
    layer->y = y;
    merge_dirty_layer_rect(&lc->dirty_rect, layer);
}

struct LayerControl* init_layer_control(unsigned char* vram, int width, int height)
{
    struct LayerControl* lc = (struct LayerControl*) malloc(sizeof(struct LayerControl));
    lc->vram = vram;
    lc->width = width;
    lc->height = height;
    lc->number_of_layers = 0;

    lc->dirty_rect.x = 0;
    lc->dirty_rect.y = 0;
    lc->dirty_rect.width = 0;
    lc->dirty_rect.height = 0;

    return lc;
}

struct Layer* layer_create(struct LayerControl* lc, int x, int y, int width, int height)
{
    int i = lc->number_of_layers;
    struct Layer* layer = &lc->layers[i];

    layer->buffer = malloc(sizeof(unsigned char) * width * height);
    layer->x = x;
    layer->y = y;
    layer->width = width;
    layer->height = height;

    lc->sorted_layers[i] = layer;
    // TODO: sort here

    lc->number_of_layers++;
    return layer;
}

void layer_flush(struct LayerControl* lc)
{
    int gx0 = lc->dirty_rect.x;
    int gx1 = lc->dirty_rect.x + lc->dirty_rect.width;
    int gy0 = lc->dirty_rect.y;
    int gy1 = lc->dirty_rect.y + lc->dirty_rect.height;

    layer_global_refresh(lc, gx0, gx1, gy0, gy1);

    lc->dirty_rect.x = 0;
    lc->dirty_rect.y = 0;
    lc->dirty_rect.width = 0;
    lc->dirty_rect.height = 0;
}
