#include <stdlib.h>
#include <unistd.h>

void _Exit(int code) {
    _exit(code);
    __builtin_unreachable();
}

void exit(int code) __attribute__((weak, alias("_Exit")));