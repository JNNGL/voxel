#include "ctype.h"

int toupper(int c) {
    if(c >= 'a' && c <= 'z') {
        return c - ('a' - 'A');
    }

    return c;
}

int isdigit(char ch) {
    return ch >= '0' && ch <= '9';
}