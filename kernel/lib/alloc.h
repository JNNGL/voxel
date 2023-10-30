#pragma once

#include <stddef.h>

void heap_init(size_t page_count);

void* malloc(size_t n);
void* realloc(void* ptr, size_t s);
void free(void* ptr);