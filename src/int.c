#include <bootpack.h>
#include <nasmfunc.h>
#include <stdio.h>

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

void initialize_ringbuffer_char(struct RingBufferChar* rb, unsigned char* buffer, int size)
{
    rb->size = size;
    rb->buffer = buffer;
    rb->free = size;
    rb->rp = 0;
    rb->wp = 0;
}

int count_ringbuffer_char(struct RingBufferChar* rb)
{
    return rb->size - rb->free;
}

int put_ringbuffer_char(struct RingBufferChar* rb, unsigned char data)
{
    if (rb->free == 0) {
        return -1;
    }
    rb->buffer[rb->wp] = data;
    rb->wp = (rb->wp + 1) % rb->size;
    rb->free--;
    return 0;
}

int get_ringbuffer_char(struct RingBufferChar* rb, unsigned char* data)
{
    if (rb->free == rb->size) {
        return -1;
    }
    *data = rb->buffer[rb->rp];
    rb->rp = (rb->rp + 1) % rb->size;
    rb->free++;
    return 0;
}

// PS/2 keyboard
// See http://oswiki.osask.jp/?%28PIC%298259A
void inthandler21(int* esp)
{
    // notify PIC to we receive interruption
    io_out8(PIC0_OCW2, 0x61);
    unsigned char data = io_in8(PORT_KEYDATA);
    put_ringbuffer_char(&keyboard_inputs, data);
}

// PS/2 mouse
void inthandler2c(int* esp)
{
    // struct BootInfo* bootInfo = (struct BootInfo*) ADDR_BOOTINFO;
    // box_fill(bootInfo->vram, bootInfo->screenWidth, 3, 0, 0, 32 * 8 - 1, 15);
    // putfont8_str(bootInfo->vram, bootInfo->screenWidth, "INT 2c (IRQ-1): PS/2 mouse", font, 5, 0, 0);
    // while(1)
    //     io_hlt();
}

void inthandler27(int *esp)
{
    // In some chipset like Athlon64X2, this interruption is called when initialized.
    // Tell PIC IRQ0-7 is ready
    io_out8(PIC0_OCW2, 0x67);
}
