#pragma once

#define NULL 0
typedef unsigned int size_t;

void init_malloc(void* address, size_t size);
void* malloc(size_t size);
void free(void* p);
size_t malloc_free_size(void);
