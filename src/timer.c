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
    io_out8(PIC0_OCW2, 0x60); // IRQ-0 is 0th pin of PIC0. 0x60 = 0x60 + 0
    check_timeout();
}

void task_switch(void);

void check_timeout()
{
    timer_control->count++;

    char do_Task_switch = 0;
    while (timer_control->sorted_timeouts != NULL && timer_control->sorted_timeouts->timeout <= timer_control->count) {
        struct Timeout* fired = timer_control->sorted_timeouts;

        if (fired == task_timer)
            do_Task_switch = 1;
        else
            put_ringbuffer_char(fired->buffer, fired->data);

        timer_control->sorted_timeouts = fired->next;
        timer_control->number_of_timeouts--;

        init_timeout(fired);
    }

    // task switch must be here, as switching tasks will change EFLAGS and it may allow interrupt.
    if (do_Task_switch)
        task_switch();
}

void init_timeout(struct Timeout* timeout)
{
    timeout->buffer = NULL;
    timeout->timeout = 1000000;
    timeout->data = 0;
    timeout->used = 0;
    timeout->next = NULL;
}

struct Timeout* get_free_timer(void)
{
    struct Timeout* timer = NULL;
    for (int i = 0; i < MAX_NUMBER_OF_TIMERS; i++) {
        if (!timer_control->timeouts[i].used) {
            timer = &timer_control->timeouts[i];
            break;
        }
    }
    return timer;
}

void insert_timer(struct Timeout* timer)
{
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
}

int set_timeout(struct RingBufferChar* buffer, unsigned char data, unsigned int ms)
{
    // Prevent timer
    int eflag = io_load_eflags();
    io_cli();

    struct Timeout* timer = get_free_timer();

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

    insert_timer(timer);

    io_store_eflags(eflag);

    return 0;
}

int task_switch_in_ms = 20;
void task_init(void)
{
    task_timer = get_free_timer();
    init_timeout(task_timer);
    task_timer->timeout = timer_control->count + (task_switch_in_ms / 10);
    task_timer->used = 1;
    current_task = 3;
    insert_timer(task_timer);
}

void task_switch(void)
{
    task_timer = get_free_timer();
    init_timeout(task_timer);
    task_timer->timeout = timer_control->count + (task_switch_in_ms / 10);
    task_timer->used = 1;

    int next_task = 0;
    if (current_task == 3)
        next_task = 4;
    else if (current_task == 4)
        next_task = 3;

    current_task = next_task;
    insert_timer(task_timer);
    farjmp(0, next_task * 8);
}
