/*
 * sys/types.h - data types.
 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_types.h.html
 */

#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H 1

#include <stddef.h>

typedef unsigned long blkcnt_t;
typedef unsigned long blksize_t;
typedef long clock_t;
typedef int clockid_t;
typedef int dev_t;
typedef unsigned long fsblkcnt_t;
typedef unsigned long fsfilcnt_t;
typedef int gid_t;
typedef int id_t;
typedef int ino_t;
typedef int key_t;
typedef int mode_t;
typedef unsigned short nlink_t;
typedef long off_t;
typedef int pid_t;
typedef long ssize_t;
typedef long suseconds_t;
typedef long time_t;
typedef int timer_t;
typedef int uid_t;

#endif