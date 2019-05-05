#include <bootpack.h>
#include <nasmfunc.h>
#include <stdlib.h>

#define PIT_CONTROL 0x0043
#define PIT_COUNT0 0x0040

void init_timeout(struct Timeout*);
void check_timeout();

void init_pit(void)
{
    timer_control = (struct TimerControl*) malloc(sizeof(struct TimerControl));
    for (int i = 0; i < MAX_NUMBER_OF_TIMERS; i++) {
        init_timeout(&timer_control->timeouts[i]);
    }
    timer_control->count = 0;
    timer_control->sorted_timeouts = NULL;
    timer_control->number_of_timeouts = 0;

    io_out8(PIT_CONTROL, 0x34);
    io_out8(PIT_COUNT0,  0x9c);
    io_out8(PIT_COUNT0,  0x2e);
}

// inthandler20 is called every 1/100 s.
void inthandler20(int* esp)
{
    check_timeout();
    io_out8(PIC0_OCW2, 0x60); // IRQ-0 is 0th pin of PIC0. 0x60 = 0x60 + 0
}

void check_timeout()
{
    timer_control->count++;
    while (timer_control->sorted_timeouts != NULL && timer_control->sorted_timeouts->timeout <= timer_control->count) {
        struct Timeout* fired = timer_control->sorted_timeouts;
        put_ringbuffer_char(fired->buffer, fired->data);
        timer_control->sorted_timeouts = fired->next;
        timer_control->number_of_timeouts--;
        init_timeout(fired);
    }
}

void init_timeout(struct Timeout* timeout)
{
    timeout->buffer = NULL;
    timeout->timeout = 1000000;
    timeout->data = 0;
    timeout->used = 0;
    timeout->next = NULL;
}

int set_timeout(struct RingBufferChar* buffer, unsigned char data, unsigned int ms)
{
    // Prevent timer
    int eflag = io_load_eflags();
    io_cli();

    struct Timeout* timer = NULL;
    for (int i = 0; i < MAX_NUMBER_OF_TIMERS; i++) {
        if (!timer_control->timeouts[i].used) {
            timer = &timer_control->timeouts[i];
            break;
        }
    }
    if (timer == NULL) {
        io_store_eflags(eflag);
        return 1;
    }

    init_timeout(timer);
    timer->buffer = buffer;
    // our timer precision is 1/100
    timer->timeout = timer_control->count + (ms / 10);
    timer->data = data;
    timer->used = 1;

    char is_inserted = 0;
    struct Timeout* prev = NULL;
    struct Timeout* next = timer_control->sorted_timeouts;
    for (int i = 0; i < timer_control->number_of_timeouts; i++) {
        if (timer->timeout < next->timeout) {
            if (i == 0) {
                // Insert as first element.
                timer->next = next;
                timer_control->sorted_timeouts = timer;
            } else {
                prev->next = timer;
                timer->next = next;
            }
            is_inserted = 1;
            break;
        }
        prev = next;
        next = next->next;
    }

    // timer goes last.
    if (!is_inserted) {
        if (timer_control->number_of_timeouts == 0) {
            timer->next = NULL;
            timer_control->sorted_timeouts = timer;
        } else {
            prev->next = timer;
            timer->next = NULL;
        }
    }
    timer_control->number_of_timeouts++;
    io_store_eflags(eflag);

    return 0;
}

