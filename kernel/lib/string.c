#include "string.h"

#include <lib/alloc.h>

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }

    return len;
}

char* strcat(char* dst, const char* src) {
    strcpy(dst + strlen(dst), src);
    return dst;
}

char* strcpy(char* dst, const char* src) {
    char* ptr = dst;
    while (*src != 0) {
        *dst++ = *src++;
    }

    *dst = 0;
    return ptr;
}

char* strdup(const char* str) {
    if (!str) {
        return 0;
    }

    size_t len = strlen(str) + 1;
    char* newstr = malloc(len);
    memcpy(newstr, str, len);
    return newstr;
}

void* memset(void* ptr, int value, size_t size) {
    asm volatile("cld; rep stosb" : "+c"(size), "+D"(ptr) : "a"(value) : "memory");
    return ptr;
}

char* strstr(const char* a, const char* b) {
    char* current = (char*) a;
    while (1) {
        char* result = current++;
        for (size_t i = 0; b[i]; i++) {
            if (!result[i]) {
                return 0;
            } else if (result[i] != b[i]) {
                result = 0;
                break;
            }
        }

        if (result) {
            return result;
        }
    }
}

char* _strstr(const char* a, const char* b, size_t limit) {
    char* current = (char*) a;
    for (size_t j = 0; j <= limit - strlen(b); j++) {
        char* result = current++;
        for (size_t i = 0; b[i]; i++) {
            if (result[i] != b[i]) {
                result = 0;
                break;
            }
        }

        if (result) {
            return result;
        }
    }

    return 0;
}

int memcmp(void* ptr1, void* ptr2, size_t size) {
    uint8_t* a = (uint8_t*) ptr1;
    uint8_t* b = (uint8_t*) ptr2;
    for (size_t i = 0; i < size; i++) {
        if (a[i] < b[i]) {
            return -1;
        } else if (a[i] > b[i]) {
            return 1;
        }
    }

    return 0;
}

char* _strchr(const char* str, int chr, uint32_t limit) {
    for (uint32_t i = 0; i < limit; i++) {
        if (str[i] == chr) {
            return (char*) (str + i);
        } else if (str[i] == 0) {
            return 0;
        }
    }

    return 0;
}

char* strchr(const char* str, int chr) {
    for (;; str++) {
        if (*str == chr) {
            return (char*) str;
        } else if (*str == 0) {
            return 0;
        }
    }
}

int strcmp(const char* str1, const char* str2) {
    const uint8_t* a = (const uint8_t*) str1;
    const uint8_t* b = (const uint8_t*) str2;
    for (size_t i = 0;; i++) {
        if (a[i] < b[i]) {
            return -1;
        } else if (a[i] > b[i]) {
            return 1;
        } else if (a[i] == 0) {
            return 0;
        }
    }
}

void* memcpy(void* dst, const void* src, size_t n) {
    asm volatile("cld; rep movsb" : "+c"(n), "+S"(src), "+D"(dst) : : "memory");
    return dst;
}