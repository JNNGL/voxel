/*
 * stddef.h - standard type definitions
 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/stddef.h.html
 */

#ifndef _STDDEF_H
#define _STDDEF_H 1

#ifdef __cplusplus
extern "C" {
#endif

#define NULL ((void*) 0)

typedef long ptrdiff_t;
typedef __WCHAR_TYPE__ wchar_t;
typedef unsigned long size_t;

#ifdef __cplusplus
}
#endif

#endif