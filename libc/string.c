#include <string.h>

#include <stdint.h>
#include <stdlib.h>

void* memcpy(void* restrict dst, const void* restrict src, size_t n) {
    asm volatile("cld; rep movsb" : "+c"(n), "+S"(src), "+D"(dst) : : "memory");
    return dst;
}

void* memmove(void* dst, const void* src, size_t n) {
    uint8_t* from = (uint8_t*) src;
    uint8_t* to = (uint8_t*) dst;

    if (from == to || n == 0) {
        return dst;
    }

    if (to > from && to - from < n) {
        for (intptr_t i = n - 1; i >= 0; i--) {
            to[i] = from[i];
        }

        return dst;
    } else if (from > to && from - to < n) {
        for (intptr_t i = 0; i < n; i++) {
            to[i] = from[i];
        }

        return dst;
    } else {
        memcpy(dst, src, n);
        return dst;
    }
}

void* memset(void* ptr, int value, size_t size) {
    asm volatile("cld; rep stosb" : "+c"(size), "+D"(ptr) : "a"(value) : "memory");
    return ptr;
}

char* strcat(char* restrict dst, const char* restrict src) {
    strcpy(dst + strlen(dst), src);
    return dst;
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

char* strcpy(char* restrict dst, const char* restrict src) {
    char* ptr = dst;
    while (*src != 0) {
        *dst++ = *src++;
    }

    *dst = 0;
    return ptr;
}

size_t strcspn(const char* str1, const char* str2) {
    const char* ptr1;
    for (ptr1 = str1; *ptr1 != 0; ++ptr1) {
        for (const char* ptr2 = str2; *ptr2 != 0; ++ptr2) {
            if (*ptr1 == *ptr2) {
                return ptr1 - str1;
            }
        }
    }
    return ptr1 - str1;
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

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }

    return len;
}

char* strpbrk(const char* str1, const char* str2) {
    str1 += strcspn(str1, str2);
    return *str1 ? (char*) str1 : 0;
}

size_t strspn(const char* str1, const char* str2) {
    if (!str2[0]) {
        return 0;
    }

    if (!str2[1]) {
        const char* ptr = str1;
        for (; *str1 == *str2; str1++);
        return str1 - ptr;
    }

    const char* ptr1;
    for (ptr1 = str1; *ptr1 != 0; ++ptr1) {
        for (const char* ptr2 = str2;; ++ptr2) {
            if (*ptr2 == 0) {
                return ptr1 - str1;
            } else if (*ptr1 == *ptr2) {
                break;
            }
        }
    }
    return ptr1 - str1;
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

char* strtok_r(char* restrict str, const char* restrict delim, char** restrict out) {
    if (!str) {
        str = *out;
    }
    str += strspn(str, delim);
    if (!*str) {
        *out = str;
        return 0;
    }
    char* token = str;
    str = strpbrk(token, delim);
    if (!str) {
        *out = strchr(token, 0);
    } else {
        *str = 0;
        *out = str + 1;
    }
    return token;
}