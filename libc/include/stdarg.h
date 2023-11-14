/*
 * stdarg.h - handle variable argument list
 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/stdarg.h.html
 */

#ifndef _STDARG_H
#define _STDARG_H 1

#ifdef __cplusplus
extern "C" {
#endif

typedef __builtin_va_list va_list;

#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap)          __builtin_va_end(ap)
#define va_arg(ap, type)    __builtin_va_arg(ap, type)
#define va_copy(d, s)       __builtin_va_copy(d, s)

#ifdef __cplusplus
}
#endif

#endif