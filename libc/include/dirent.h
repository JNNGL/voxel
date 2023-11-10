/*
 * dirent.h - format of directory entries
 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/dirent.h.html
 */

#ifndef _DIRENT_H
#define _DIRENT_H 1

#include <sys/types.h>

struct dirent {
    ino_t d_ino;
    char d_name[256];
};

typedef struct _DIR DIR;

// int alphasort(const struct dirent**, const struct dirent**);
int closedir(DIR* dir);
int dirfd(DIR*);
DIR* fdopendir(int fd);
DIR* opendir(const char* name);
struct dirent* readdir(DIR*);
// int readdir_r(DIR* restrict, struct dirent* restrict, struct dirent** restrict);
// void rewinddir(DIR*);
// int scandir(const char*, struct dirent***, int(*)(const struct dirent*), int(*)(const struct dirent**, const struct dirent**));
// void seekdir(DIR*, long);
// long telldir(DIR*);

#endif