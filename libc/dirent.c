#include <sys/syscall.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>

struct _DIR {
    int fd;
    int index;
};

int closedir(DIR* dir) {
    if (dir && (dir->fd != -1)) {
        return close(dir->fd);
    } else {
        return -EBADF;
    }
}

int dirfd(DIR* dir) {
    return dir->fd;
}

DIR* fdopendir(int fd) {
    DIR* dir = malloc(sizeof(DIR));
    dir->fd = fd;
    dir->index = 0;
    return dir;
}

DIR* opendir(const char* name) {
    int fd = open(name, O_RDONLY);
    if (fd < 0) {
        return NULL;
    }

    return fdopendir(fd);
}

struct dirent* readdir(DIR* dir) {
    static struct dirent _dirent;
    long out = syscall_readdir(dir->fd, dir->index++, &_dirent);
    if (out < 0) {
        errno = -out;
        return NULL;
    }

    if (out == 0) {
        return NULL;
    }

    return &_dirent;
}