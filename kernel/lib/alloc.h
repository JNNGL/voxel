#pragma once

#include <stddef.h>

void heap_init(size_t page_count);

void* malloc(size_t n);
void free(void* ptr);