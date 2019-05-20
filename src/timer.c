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

    char do_task_switch = 0;
    while (timer_control->sorted_timeouts != NULL && timer_control->sorted_timeouts->timeout <= timer_control->count) {
        struct Timeout* fired = timer_control->sorted_timeouts;

        if (fired == task_timer)
            do_task_switch = 1;
        else
            put_ringbuffer_char(fired->buffer, fired->data);

        timer_control->sorted_timeouts = fired->next;
        timer_control->number_of_timeouts--;

        // Prevent set task_timer not-used.
        if (fired != task_timer)
            init_timeout(fired);
    }

    // task switch must be here, as switching tasks will change EFLAGS and it may allow interrupt.
    if (do_task_switch)
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

struct Task* get_free_task(void)
{
    struct Task* ret = NULL;

    for (int i = 0; i < MAX_NUMBER_OF_TIMERS; i++) {
        if (!(task_control->tasks[i].flags & TASK_FLAG_USED)) {
            ret = &task_control->tasks[i];
            ret->flags |= TASK_FLAG_USED;
            ret->tss.eflags = 0x00000202; // IF = 1, default value?
            // Caller should set up these values.
            ret->tss.eax = 0;
            ret->tss.ecx = 0;
            ret->tss.edx = 0;
            ret->tss.ebx = 0;
            ret->tss.esp = 0;
            ret->tss.ebp = 0;
            ret->tss.edi = 0;
            ret->tss.es = 0;
            ret->tss.cs = 0; // bootpack's code segment
            ret->tss.ss = 0;
            ret->tss.ds = 0;
            ret->tss.fs = 0;
            ret->tss.gs = 0;
            ret->tss.ldtr = 0;
            ret->tss.iomap = 0x40000000; // FIXME: What this value means?

            return ret;
        }
    }
    return NULL;
}

// Just provide another name
struct Task* task_new(void)
{
    return get_free_task();
}

struct Task* task_init(void)
{
    struct SegmentDescriptor* segmentDescriptors = (struct SegmentDescriptor*) ADDR_GDT;

    task_control = (struct TaskControl*) malloc(sizeof(struct TaskControl));
    task_control->number_of_tasks = 0;
    for (int i = 0; i < MAX_NUMBER_OF_TASKS; i++) {
        task_control->tasks[i].selector = INITIAL_TASK_SELECTOR + i;
        task_control->tasks[i].flags = TASK_FLAG_NONE;
        set_segment_descriptor(&segmentDescriptors[INITIAL_TASK_SELECTOR + i], 103, (int) &task_control->tasks[i].tss, AR_TSS32);
    }

    struct Task* current_task = get_free_task();
    load_tr(current_task->selector * 8);
    task_start(current_task);

    // Start task switcher.
    task_timer = get_free_timer();
    init_timeout(task_timer);
    task_timer->timeout = timer_control->count + (TASK_SWITCH_TIMEFRAME_IN_MS / 10);
    task_timer->used = 1;
    insert_timer(task_timer);

    return current_task;
}

void task_switch(void)
{
    // Setup next switcher.
    task_timer->timeout = timer_control->count + (TASK_SWITCH_TIMEFRAME_IN_MS / 10);
    insert_timer(task_timer);

    // Jump to next task
    // FIXME: jumping to the same task is not supported, should check selector.
    if (task_control->number_of_running_tasks > 1) {
        task_control->current_task_index = (task_control->current_task_index + 1) % task_control->number_of_running_tasks;
        farjmp(0, task_control->running[task_control->current_task_index]->selector * 8);
    }
}

void task_start(struct Task* task)
{
    int index = task_control->number_of_running_tasks;
    task_control->running[index] = task;
    task_control->number_of_running_tasks++;
}
