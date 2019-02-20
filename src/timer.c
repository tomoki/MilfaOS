#include <bootpack.h>
#include <nasmfunc.h>

#define PIT_CONTROL 0x0043
#define PIT_COUNT0 0x0040

void init_pit(void)
{
    io_out8(PIT_CONTROL, 0x34);
    io_out8(PIT_COUNT0,  0x9c);
    io_out8(PIT_COUNT0,  0x2e);
}

void inthandler20(int* esp)
{
    io_out8(PIC0_OCW2, 0x60); // IRQ-0 is 0th pin of PIC0. 0x60 = 0x60 + 0
    timer_data.count++;
}
