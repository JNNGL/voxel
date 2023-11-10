#pragma once

#include <cpu/idt.h>

#define	SYS_EXIT      0
#define	SYS_FORK      1
#define	SYS_READ      2
#define	SYS_WRITE     3
#define	SYS_OPEN      4
#define	SYS_CLOSE     5
#define SYS_SEEK      6
#define SYS_GETPID    7
#define SYS_SBRK      8
#define SYS_UNAME     9
#define SYS_STAT      10
#define SYS_STATF     11
#define SYS_LSTAT     12
#define SYS_READDIR   13
#define SYS_MKDIR     14
#define SYS_IOCTL     15
#define SYS_ACCESS    16
#define SYS_CHMOD     17
#define SYS_UMASK     18
#define SYS_CHOWN     19
#define SYS_UNLINK    20
#define SYS_SYMLINK   21
#define SYS_READLINK  22
#define SYS_DUP2      23
#define SYS_EXECVE    24
#define SYS_WAITPID   25

void syscall_handler(struct regs* r);