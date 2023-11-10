/*
 * unistd.h - standard symbolic constants and types
 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/unistd.h.html
 */

#ifndef _UNISTD_H
#define _UNISTD_H 1

#include <sys/types.h>
#include <stdio.h>

#define _POSIX_VERSION 200809L
#define _POSIX2_VERSION 200809L
#define _XOPEN_VERSION 700

#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_OK 0

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

extern char* optarg;
extern int opterr, optind, optopt;

extern char** environ;

int access(const char* path, int mode);
// unsigned alarm(unsigned);
// int chdir(const char*);
int chown(const char* path, uid_t owner, gid_t group);
int close(int fd);
// size_t confstr(int, char*, size_t);
// char* crypt(const char*, const char*);
int dup(int fd);
int dup2(int oldfd, int newfd);
void _exit(int code);
// void encrypt(char[64], int);
// int execl(const char*, const char*, ...);
// int execle(const char*, const char*, ...);
// int execlp(const char*, const char*, ...);
int execv(const char* path, char* const argv[]);
int execve(const char* path, char* const argv[], char* const envp[]);
// int execvp(const char*, char* const[]);
// int faccessat(int, const char*, int, int);
// int fchdir(int);
// int fchown(int, uid_t, gid_t);
// int fchownat(int, const char*, uid_t, gid_t, int);
// int fdatasync(int);
// int fexecve(int, char* const[], char* const[]);
pid_t fork();
// long fpathconf(int, int);
// int fsync(int);
// int ftruncate(int, off_t);
// char* getcwd(char*, size_t);
// gid_t getegid();
// uid_t geteuid();
// gid_t getgid();
// int getgroups(int, gid_t[]);
// long gethostid();
// int gethostname(char*, size_t);
// char* getlogin(void);
// int getlogin_r(char*, size_t);
// int getopt(int, char* const[], const char*);
// pid_t getpgid(pid_t);
// pid_t getpgrp();
pid_t getpid();
pid_t getppid();
// pid_t getsid(pid_t);
// uid_t getuid();
// int isatty(int);
// int lchown(const char*, uid_t, gid_t);
// int link(const char*, const char*);
// int linkat(int, const char*, int, const char*, int);
// int lockf(int, int, off_t);
off_t lseek(int fd, off_t off, int whence);
// int nice(int);
// long pathconf(const char*, int);
// int pause();
// int pipe(int[2]);
// ssize_t pread(int, void*, size_t, off_t);
// ssize_t pwrite(int, const void*, size_t, off_t);
ssize_t read(int fd, void* ptr, size_t len);
ssize_t readlink(const char* restrict name, char* restrict buf, size_t len);
// ssize_t readlinkat(int, const char* restrict, char* restrict, size_t);
int rmdir(const char* name);
// int setegid(gid_t);
// int seteuid(uid_t);
// int setgid(gid_t);
// int setpgid(pid_t, pid_t);
// pid_t setpgrp();
// int setregid(gid_t, gid_t);
// int setreuid(uid_t, uid_t);
// pid_t setsid();
// int setuid(uid_t);
// unsigned sleep(unsigned);
// void swab(const void* restrict, void* restrict, ssize_t);
int symlink(const char* target, const char* name);
// int symlinkat(const char*, int, const char*);
// void sync();
// long sysconf(int);
// pid_t tcgetpgrp(int);
// int tcsetpgrp(int, pid_t);
// int truncate(int, pid_t);
// char* ttyname(int);
// int ttyname_r(int, char*, size_t);
int unlink(const char* path);
// int unlinkat(int, const char*, int);
ssize_t write(int fd, const void* ptr, size_t len);

#endif