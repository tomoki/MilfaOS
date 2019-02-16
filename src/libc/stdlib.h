#pragma once

#define NULL 0
typedef unsigned int size_t;

void init_malloc(void* address, size_t size);
void* malloc(size_t size);
void free(void* p);
size_t malloc_free_size(void);

// returns last address which has memory
size_t memtest(size_t start, size_t end);
