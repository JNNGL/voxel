#include <sys/syscall.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

void _Exit(int code) {
    _exit(code);
    __builtin_unreachable();
}

void abort() {
    syscall_exit(1); // TODO
}

int abs(int x) {
    return x < 0 ? -x : x;
}

int atexit(void(*func)(void)) {
    return -EINVAL; // TODO
}

int64_t _atoi(const char* buf, uint32_t radix) {
    int toupper(int c) { // TODO: ctype.h
        if(c >= 'a' && c <= 'z') {
            return c - ('a' - 'A');
        }

        return c;
    }

    int isdigit(char ch) {
        return ch >= '0' && ch <= '9';
    }

    int64_t res = 0;
    bool sign = 0;
    if (radix < 2 || radix > 36) {
        return -4;
    }

    char* ptr = (char*) buf;
    char c;
    static const char* alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    while ((c = *ptr++) != 0) {
        if (c == '-') {
            sign = 1;
            continue;
        }

        if (strchr("\t\n\v\f\r +_", c) != NULL) {
            continue;
        }

        char* x = strchr(alphabet, toupper(c));
        if (x == 0) {
            break;
        }

        size_t pos = x - alphabet;
        if (pos >= radix) {
            break;
        }

        res = res * radix + pos;
    }

    return sign ? -res : res;
}

int atoi(const char* s) {
    return (int) _atoi(s, 10);
}

void exit(int code) __attribute__((weak, alias("_Exit")));

char* getenv(const char* s) {
    return NULL; // TODO
}