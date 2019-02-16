#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

int int2str(char* str, int n)
{
    int minus = n < 0;
    if (minus) {
        str[0] = '-';
        str++;
        n *= -1;
    }
    // calculate how many numbers n has
    int a = n;
    int k = 1;
    while (a >= 10) {
        a /= 10;
        k++;
    }

    // insert characters backward
    int ret = k;
    if (minus)
        ret++; // -

    while (k > 0) {
        str[k-1] = '0' + (n % 10);
        n /= 10;
        k--;
    }
    return ret;
}

int sprintf(char * restrict s, char* restrict format, ...)
{
    va_list args;
    va_start(args, format);
    int orig_s = (int) s;
    while (*format != '\0') {
        if (*format == '%') {
            format++;
            if (*format == '%') {
                *(s++) = '%';
                format++;
            } else if(*format == 'd') {
                int* n = (int*) args;
                s += int2str(s, *n);
                format++;
                n++;
                args = (void*) n;
            }
        } else {
            *s = *format;
            s++;
            format++;
        }
    }
    *s = '\0';
    int ret = ((int) s) - orig_s;
    va_end(args);
    return ret;
}

// memory managament.
#define NUMBER_OF_MELLOC_CHUNKS 4000
#define MALLOC_INFO_ADDR 0x003c0000

struct MallocChunk {
    void* address;
    size_t size;
    char is_free;
    char is_using;
    struct MallocChunk* next;
};

struct MallocInfo {
    struct MallocChunk chunks[NUMBER_OF_MELLOC_CHUNKS];
    struct MallocChunk* list;
};

void malloc_init_chunk(struct MallocChunk* chunk)
{
    chunk->address = NULL;
    chunk->size = 0;
    chunk->is_free = 0;
    chunk->next = NULL;

    // meta
    chunk->is_using = 0;
}

struct MallocChunk* malloc_get_chunk(void)
{
    struct MallocInfo* info = (struct MallocInfo*) MALLOC_INFO_ADDR;
    for (int i = 0; i < NUMBER_OF_MELLOC_CHUNKS; i++) {
        struct MallocChunk* p = &info->chunks[i];
        if (!p->is_using) {
            malloc_init_chunk(p);
            p->is_using = 1;
            return p;
        }
    }
    return NULL;
}

void malloc_remove_chunk(struct MallocChunk* p)
{
    malloc_init_chunk(p);
}

void init_malloc(void* address, size_t size)
{
    struct MallocInfo* info = (struct MallocInfo*) MALLOC_INFO_ADDR;

    for (int i = 0; i < NUMBER_OF_MELLOC_CHUNKS; i++)
        malloc_init_chunk(&info->chunks[i]);

    info->list = malloc_get_chunk();
    info->list->address = address;
    info->list->size = size;
    info->list->is_free = 1;
    info->list->next = NULL;
}

void* malloc(size_t size)
{
    struct MallocInfo* info = (struct MallocInfo*) MALLOC_INFO_ADDR;

    for (struct MallocChunk* p = info->list; p != NULL; p = p->next) {
        if (p->is_free) {
            if (p->size > size) {
                struct MallocChunk* next = malloc_get_chunk();
                struct MallocChunk* next_next = p->next;

                next->size = p->size - size;
                p->size = size;
                next->address = (void*)((size_t)p->address + p->size);

                p->next = next;
                next->next  = next_next;

                p->is_free = 0;
                next->is_free = 1;

                return p->address;
            } else if(p->size == size) {
                p->is_free = 0;
                return p->address;
            }
        }
    }

    return NULL;
}

void free(void* fp)
{
    struct MallocInfo* info = (struct MallocInfo*) MALLOC_INFO_ADDR;

    for (struct MallocChunk* p = info->list; p != NULL; p = p->next) {
        if (p->address == fp)
            p->is_free = 1;
    }
    // FIXME: Do this more efficient way by merging two loops
    // Merge free blocks
    for (struct MallocChunk* p = info->list; p != NULL; p = p->next) {
        while (1) {
            struct MallocChunk* n = p->next;
            if (p->is_free && n && n->is_free) {
                p->size += n->size;
                p->next = n->next;
                malloc_init_chunk(n);
            } else
                break;
        }
    }
}

size_t malloc_free_size(void)
{
    struct MallocInfo* info = (struct MallocInfo*) MALLOC_INFO_ADDR;
    size_t ret = 0;
    for (struct MallocChunk* p = info->list; p != NULL; p = p->next) {
        if (p->is_free)
            ret += p->size;
    }
    return ret;
}
