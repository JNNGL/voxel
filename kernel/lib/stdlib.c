#include "stdlib.h"

#include <lib/string.h>
#include <lib/ctype.h>
#include <stddef.h>
#include <stdbool.h>

size_t memrev(char* buf, size_t n) {
    size_t i;
    for (i = 0; i < n / 2; ++i) {
        const char x = buf[i];
        buf[i] = buf[n - i - 1];
        buf[n - i - 1] = x;
    }

    return i;
}

size_t strrev(char* s) {
    return memrev(s, strlen(s));
}

char* itoa(int64_t target, char* buf, uint32_t radix) {
    if (radix < 2 || radix > 36) {
        return 0;
    }

    int sign = 0;
    if (target < 0) {
        sign = 1;
        target = -target;
    }

    char* low = buf;
    do {
        *buf++ = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[target % radix];
    } while (target /= radix);

    if (sign) {
        *buf++ = '-';
    }

    *buf = '\0';
    strrev(low);
    return buf;
}

int64_t atoi(const char* buf, uint32_t radix) {
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