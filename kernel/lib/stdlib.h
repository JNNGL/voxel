#pragma once

#include <stdint.h>

char* itoa(int64_t target, char* buf, uint32_t radix);
int64_t atoi(const char* buf, uint32_t radix);