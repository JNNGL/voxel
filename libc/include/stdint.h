/*
 * stdint.h - integer types
 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/stdint.h.html
 */

#ifndef _STDINT_H
#define _STDINT_H 1

#ifdef __cplusplus
extern "C" {
#endif

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;

typedef int8_t int_least8_t;
typedef uint8_t uint_least8_t;
typedef int16_t int_least16_t;
typedef uint16_t uint_least16_t;
typedef int32_t int_least32_t;
typedef uint32_t uint_least32_t;
typedef int64_t int_least64_t;
typedef uint64_t uint_least64_t;

typedef int8_t int_fast8_t;
typedef uint8_t uint_fast8_t;
typedef int16_t int_fast16_t;
typedef uint16_t uint_fast16_t;
typedef int32_t int_fast32_t;
typedef uint32_t uint_fast32_t;
typedef int64_t int_fast64_t;
typedef uint64_t uint_fast64_t;

typedef long intptr_t;
typedef unsigned long uintptr_t;

typedef int64_t intmax_t;
typedef uint64_t uintmax_t;

typedef long ptrdiff_t;

#define INT8_MIN -128
#define INT8_MAX 127
#define INT_LEAST8_MIN INT8_MIN
#define INT_LEAST8_MAX INT8_MAX
#define INT_FAST8_MIN INT8_MIN
#define INT_FAST8_MAX INT8_MAX

#define UINT8_MAX 255
#define UINT_LEAST8_MAX UINT8_MAX
#define UINT_FAST8_MAX UINT8_MAX

#define INT16_MIN -32768
#define INT16_MAX 32767
#define INT_LEAST16_MIN INT16_MIN
#define INT_LEAST16_MAX INT16_MAX
#define INT_FAST16_MIN INT16_MIN
#define INT_FAST16_MAX INT16_MAX

#define UINT16_MAX 65535
#define UINT_LEAST16_MAX UINT16_MAX
#define UINT_FAST16_MAX UINT16_MAX

#define INT32_MIN -2147483648
#define INT32_MAX 2147483647
#define INT_LEAST32_MIN INT32_MIN
#define INT_LEAST32_MAX INT32_MAX
#define INT_FAST32_MIN INT32_MIN
#define INT_FAST32_MAX INT32_MAX

#define UINT32_MAX 4294967295U
#define UINT_LEAST32_MAX UINT32_MAX
#define UINT_FAST32_MAX UINT32_MAX

#define INT64_MIN -9223372036854775808LL
#define INT64_MAX 9223372036854775807LL
#define INT_LEAST64_MIN INT64_MIN
#define INT_LEAST64_MAX INT64_MAX
#define INT_FAST64_MIN INT64_MIN
#define INT_FAST64_MAX INT64_MAX

#define UINT64_MAX 18446744073709551615ULL
#define UINT_LEAST64_MAX UINT64_MAX
#define UINT_FAST64_MAX UINT64_MAX

#define INTMAX_MIN INT64_MIN
#define INTMAX_MAX INT64_MAX
#define UINTMAX_MAX UINT64_MAX

#if __LONG_WIDTH__ == 32
#define SIZE_MAX UINT32_MAX
#else
#define SIZE_MAX UINT64_MAX
#endif

#define SIG_ATOMIC_MIN (-__INT_MAX__ - 1)
#define SIG_ATOMIC_MAX __INT_MAX__

#define PTRDIFF_MAX __LONG_MAX__
#define PTRDIFF_MIN (-PTRDIFF_MAX - 1)

#ifdef __WCHAR_MAX__
#define WCHAR_MAX __WCHAR_MAX__
#endif
#ifdef __WCHAR_MIN__
#define WCHAR_MIN __WCHAR_MIN__
#endif

#ifdef __WINT_MAX__
#define WINT_MAX __WINT_MAX__
#endif
#ifdef __WINT_MIN__
#define WINT_MIN __WINT_MIN__
#else
#define WINT_MIN 0
#endif

#define INT8_C(x) x
#define UINT8_C(x) x##U
#define INT16_C(x) x
#define UINT16_C(x) x##U
#define INT32_C(x) x
#define UINT32_C(x) x##U
#define INT64_C(x) x##LL
#define UINT64_C(x) x##ULL
#define INTMAX_C(x) x##LL
#define UINTMAX_C(x) x##ULL

#ifdef __cplusplus
}
#endif

#endif