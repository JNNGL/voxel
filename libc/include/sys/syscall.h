#pragma once

#include <sys/types.h>
#include <stddef.h>

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

long syscall_exit(int code);
long syscall_fork();
long syscall_read(int fd, char* buf, size_t size);
long syscall_write(int fd, char* buf, size_t size);
long syscall_open(const char* file, int flags, int mode);
long syscall_close(int fd);
long syscall_seek(int fd, long offset, long whence);
long syscall_getpid();
long syscall_sbrk(ssize_t size);
long syscall_uname(void* name);
long syscall_stat(int fd, void* st);
long syscall_statf(const char* file, void* st);
long syscall_lstat(const char* file, void* st);
long syscall_readdir(int fd, size_t index, void* dirent);
long syscall_mkdir(const char* path, mode_t mode);
long syscall_ioctl(int fd, long cmd, void* arg);
long syscall_access(const char* file, int flags);
long syscall_chmod(const char* file, mode_t mode);
long syscall_umask(mode_t mode);
long syscall_chown(const char* file, uid_t uid, gid_t gid);
long syscall_unlink(const char* file);
long syscall_symlink(const char* target, const char* name);
long syscall_readlink(const char* file, char* ptr, size_t len);
long syscall_dup2(long oldfd, long newfd);
long syscall_execve(const char* file, char* const argv[], char* const envp[]);
long syscall_waitpid(pid_t pid);
