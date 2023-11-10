/*
 * string.h - string operations
 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/string.h.html
 */

#ifndef _STRING_H
#define _STRING_H 1

#include <stddef.h>

//void* memccpy(void* restrict, const void* restrict, int, size_t);
//void* memchr(const void*, int, size_t);
//int memcmp(const void*, const void*, size_t);
void* memcpy(void* restrict dst, const void* restrict src, size_t n);
void* memmove(void* dst, const void* src, size_t n);
void* memset(void* ptr, int value, size_t size);
//char* stpcpy(char* restrict, const char* restrict);
//char* stpncpy(char* restrict, const char* restrict, size_t);
char* strcat(char* restrict dst, const char* restrict src);
char* strchr(const char* str, int chr);
int strcmp(const char* str1, const char* str2);
//int strcoll(const char*, const char*);
char* strcpy(char* restrict dst, const char* restrict src);
size_t strcspn(const char* str1, const char* str2);
char* strdup(const char* str);
char* strerror(int err);
//int strerror_r(int, char*, size_t);
size_t strlen(const char* str);
//char* strncat(char* restrict, const char* restrict, size_t);
//int strncmp(const char*, const char*, size_t);
//char* strncpy(char* restrict, const char* restrict, size_t);
//char* strndup(const char*, size_t);
//size_t strnlen(const char*, size_t);
char* strpbrk(const char* str1, const char* str2);
//char* strrchr(const char*, int);
//char* strsignal(int);
size_t strspn(const char* str1, const char* str2);
char* strstr(const char* str1, const char* str2);
//char* strtok(char* restrict, const char* restrict);
char* strtok_r(char* restrict str, const char* restrict delim, char** restrict out);
//size_t strxfrm(char* restrict, const char* restrict, size_t);

#endif