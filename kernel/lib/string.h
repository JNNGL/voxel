#pragma once

#include <stdint.h>
#include <stddef.h>

size_t strlen(const char* str);
void* memset(void* data, int value, size_t size);
char* _strchr(const char* str, int chr, uint32_t limit);
char* strchr(const char* str, int chr);
char* _strstr(const char* a, const char* b, size_t limit);
char* strstr(const char* a, const char* b);
char* strcat(char* dst, const char* src);
char* strcpy(char* dst, const char* src);
char* strdup(const char* str);
int memcmp(void* a, void* b, size_t size);
int strcmp(const char* a, const char* b);
void* memcpy(void* dst, const void* src, size_t n);