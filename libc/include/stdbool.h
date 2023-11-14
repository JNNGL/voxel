/*
 * stdbool.h - boolean type and values
 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/stdbool.h.html
 */

#ifndef _STDBOOL_H
#define _STDBOOL_H 1

#ifdef __cplusplus
extern "C" {
#endif

#if !__bool_true_false_are_defined
#define bool  _Bool
#define true  1
#define false 0
#define __bool_true_false_are_defined 1
#endif

#ifdef __cplusplus
}
#endif

#endif