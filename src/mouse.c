#include <bootpack.h>
#include <nasmfunc.h>

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

void enable_mouse(struct MouseData* data)
{
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYCOMMAND, KEYCOMMAND_SENDTO_MOUSE);
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYDATA, MOUSECOMMAND_ENABLE);
    data->phase = 0;
}

// See https://wiki.osdev.org/PS/2_Mouse
// #define MOUSE_LEFT_CLICK (1 << 0)
// #define MOUSE_RIGHT_CLICK (1 << 1)
// #define MOUSE_CENTER_CLICK (1 << 2)
#define MOUSE_REVERSED_ONE (1 << 3)
#define MOUSE_X_AXIS_SIGN (1 << 4)
#define MOUSE_Y_AXIS_SIGN (1 << 5)
#define MOUSE_X_AXIS_OFFSET (1 << 6)
#define MOUSE_Y_AXIS_OFFSET (1 << 7)

int decode_mouse(struct MouseData* data, unsigned int next_data)
{
    if (data->phase == 0) {
        if (next_data == 0xfa)
            data->phase = 1;
        return 0;
    } else if (data->phase == 1) {
        // validation
        if ((next_data & 0xc8) == 0x08) {
            data->buf[0] = next_data;
            data->phase = 2;
        }
        return 0;
    } else if (data->phase == 2) {
        data->buf[1] = next_data;
        data->phase = 3;
    } else if (data->phase == 3) {
        data->buf[2] = next_data;
        data->phase = 1;
        data->button = data->buf[0] & (MOUSE_LEFT_CLICK | MOUSE_RIGHT_CLICK | MOUSE_CENTER_CLICK);
        data->x = data->buf[1];
        if (data->buf[0] & MOUSE_X_AXIS_SIGN)
            data->x |= 0xffffff00;

        data->y = data->buf[2];
        if (data->buf[0] & MOUSE_Y_AXIS_SIGN)
            data->y |= 0xffffff00;

        data->y *= -1;
        return 1;
    }
    return 0;
}
