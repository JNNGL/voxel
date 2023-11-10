/*
 * limits.h - implementation-defined constants
 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/limits.h.html
 */

#ifndef _LIMITS_H
#define _LIMITS_H

#define _LITTLE_ENDIAN

#define PAGESIZE  4096
#define PAGE_SIZE 4096

#define _POSIX_CLOCKRES_MIN                 20000000
#define _POSIX_ARG_MAX                      4096
#define _POSIX_CHILD_MAX                    25
#define _POSIX_DELAYTIMER_MAX               32
#define _POSIX_HOST_NAME_MAX                255
#define _POSIX_LINK_MAX                     8
#define _POSIX_LOGIN_NAME_MAX               9
#define _POSIX_MAX_CANON                    255
#define _POSIX_MAX_INPUT                    255
#define _POSIX_MQ_OPEN_MAX                  8
#define _POSIX_MQ_PRIO_MAX                  32
#define _POSIX_NAME_MAX                     14
#define _POSIX_NGROUPS_MAX                  8
#define _POSIX_OPEN_MAX                     20
#define _POSIX_PATH_MAX                     256
#define _POSIX_PIPE_BUF                     512
#define _POSIX_RE_DUP_MAX                   255
#define _POSIX_RTSIG_MAX                    8
#define _POSIX_SEM_NSEMS_MAX                256
#define _POSIX_SEM_VALUE_MAX                32767
#define _POSIX_SIGQUEUE_MAX                 32
#define _POSIX_SSIZE_MAX                    32767
#define _POSIX_SS_REPL_MAX                  4
#define _POSIX_STREAM_MAX                   8
#define _POSIX_SYMLINK_MAX                  255
#define _POSIX_SYMLOOP_MAX                  8
#define _POSIX_THREAD_DESTRUCTOR_ITERATIONS 4
#define _POSIX_THREAD_KEYS_MAX              128
#define _POSIX_THREAD_THREADS_MAX           64
#define _POSIX_TIMER_MAX                    32
#define _POSIX_TRACE_EVENT_NAME_MAX         30
#define _POSIX_TRACE_NAME_MAX               8
#define _POSIX_TRACE_SYS_MAX                8
#define _POSIX_TRACE_USER_EVENT_MAX         32
#define _POSIX_TTY_NAME_MAX                 9
#define _POSIX_TZNAME_MAX                   6

#define _POSIX2_BC_BASE_MAX        99
#define _POSIX2_BC_DIM_MAX         2048
#define _POSIX2_BC_SCALE_MAX       99
#define _POSIX2_BC_STRING_MAX      1000
#define _POSIX2_CHARCLASS_NAME_MAX 14
#define _POSIX2_COLL_WEIGHTS_MAX   2
#define _POSIX2_EXPR_NEST_MAX      32
#define _POSIX2_LINE_MAX           2048
#define _POSIX2_RE_DUP_MAX         255

#define _XOPEN_IOV_MAX   16
#define _XOPEN_NAME_MAX  255
#define _XOPEN_PATH_MAX  1024

#define CHAR_BIT   8
#define CHAR_MAX   __SCHAR_MAX__
#define CHAR_MIN   (-__SCHAR_MAX__ - 1)
#define INT_MAX    __INT_MAX__
#define INT_MIN    (-__INT_MAX__ - 1)
#define LLONG_MAX  __LONG_LONG_MAX__
#define LLONG_MIN  (-__LONG_LONG_MAX__ - 1)
#define LONG_BIT   __LONG_WIDTH__
#define LONG_MAX   __LONG_MAX__
#define LONG_MIN   (-__LONG_MAX__ - 1)
#define MB_LEN_MAX 1
#define SCHAR_MAX  127
#define SCHAR_MIN  -128
#define SHRT_MAX   __SHRT_MAX__
#define SHRT_MIN   (-__SHRT_MAX__ - 1)
#define SSIZE_MAX  __LONG_MAX__
#define UCHAR_MAX  255
#define UINT_MAX   4294967295U
#define ULLONG_MAX 18446744073709551615ULL
#if __LONG_WIDTH__ == 32
#define ULONG_MAX  UINT_MAX
#else
#define ULONG_MAX  ULLONG_MAX
#endif
#define USHRT_MAX  65535
#define WORD_BIT   __INT_WIDTH__

#endif