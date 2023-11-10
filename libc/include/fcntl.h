/*
 * fcntl.h - file control options
 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/fcntl.h.html
 */

#ifndef _FCNTL_H
#define _FCNTL_H 1

#include <sys/stat.h>
#include <unistd.h>

#define F_DUPFD  0
#define F_GETFD  1
#define F_SETFD  2
#define F_GETFL  3
#define F_SETFL  4
#define F_GETLK  5
#define F_SETLK  6
#define F_SETLKW 7
#define F_GETOWN 8
#define F_SETOWN 9

#define FD_CLOEXEC 1

#define F_RDLCK 1
#define F_UNLCK 2
#define F_WRLCK 3

#define O_RDONLY    0x0000
#define O_WRONLY    0x0001
#define O_RDWR      0x0002
#define O_APPEND    0x0008
#define O_CREAT     0x0200
#define O_TRUNC     0x0400
#define O_EXCL      0x0800
#define O_NOFOLLOW  0x1000
#define O_PATH      0x2000
#define O_NONBLOCK  0x4000
#define O_DIRECTORY 0x8000

struct flock {
    short l_type;
    short l_whence;
    off_t l_start;
    off_t l_len;
    pid_t l_pid;
};

int creat(const char* path, mode_t mode);
// int fcntl(int, int, ...);
int open(const char* path, int flags, ...);
// int openat(int, const char*, int, ...);

#endif