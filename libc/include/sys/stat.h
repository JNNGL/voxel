/*
 * sys/stat.h - data returned by the stat() function
 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_stat.h.html
 */

#ifndef _SYS_STAT_H
#define _SYS_STAT_H 1

#include <sys/types.h>
// TODO: #include <time.h>

struct stat {
    dev_t st_dev;
    ino_t st_ino;;
    mode_t st_mode;
    nlink_t st_nlink;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    off_t st_size;
    time_t st_atim;
    time_t st_mtim;
    time_t st_ctim;
    blksize_t st_blksize;
    blkcnt_t st_blocks;
};

#define S_IFMT   0170000
#define S_IFBLK  0060000
#define S_IFCHR  0020000
#define S_IFIFO  0010000
#define S_IFREG  0100000
#define S_IFDIR  0040000
#define S_IFLNK  0120000
#define S_IFSOCK 0140000

#define	S_IRWXU (S_IRUSR | S_IWUSR | S_IXUSR)
#define	S_IRUSR	0000400
#define	S_IWUSR	0000200
#define	S_IXUSR 0000100
#define	S_IRWXG	(S_IRGRP | S_IWGRP | S_IXGRP)
#define	S_IRGRP	0000040
#define	S_IWGRP	0000020
#define	S_IXGRP 0000010
#define	S_IRWXO	(S_IROTH | S_IWOTH | S_IXOTH)
#define	S_IROTH	0000004
#define	S_IWOTH	0000002
#define	S_IXOTH 0000001

#define	S_ISUID	 0004000
#define	S_ISGID	 0002000
#define	S_ISVTX	 0001000
#define	S_IREAD	 0000400
#define	S_IWRITE 0000200
#define	S_IEXEC	 0000100
#define	S_ENFMT  0002000

#define	S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)
#define	S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
#define	S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define	S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#define	S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#define	S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#define	S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)

#define S_BLKSIZE 1024

int chmod(const char* file, mode_t mode);
// int fchmod(int, mode_t);
// int fchmodat(int, const char*, mode_t, int);
int fstat(int fd, struct stat* stat);
// int fstatat(int, const char* restrict, struct stat* restrict, int);
// int futimens(int, const time_t[2]);
int lstat(const char* restrict file, struct stat* restrict stat);
int mkdir(const char* name, mode_t mode);
// int mkdirat(int, const char*, mode_t);
// int mkfifo(const char*, mode_t);
// int mkfifoat(int, const char*, mode_t);
// int mknod(const char*, mode_t, dev_t);
// int mknodat(int, const char*, mode_t, dev_t);
int stat(const char* restrict file, struct stat* restrict stat);
mode_t umask(mode_t mode);
// int utimensat(int, const char*, const time_t[2], int);

#endif