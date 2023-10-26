#pragma once

#include <stdarg.h>
#include <lib/terminal.h>

int kvprintf(const char* format, va_list list);

__attribute__((format(printf, 1, 2)))
int kprintf(const char* format, ...);