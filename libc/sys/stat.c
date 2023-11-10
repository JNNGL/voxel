#include <sys/stat.h>

#include <sys/syscall.h>
#include <errno.h>

int chmod(const char* file, mode_t mode) {
    long out = syscall_chmod(file, mode);
    if (out < 0) {
        errno = -out;
        return -1;
    }

    return out;
}

int fstat(int fd, struct stat* stat) {
    long out = syscall_stat(fd, stat);
    if (out < 0) {
        errno = -out;
        return -1;
    }

    return out;
}

int lstat(const char* restrict file, struct stat* restrict stat) {
    long out = syscall_lstat(file, stat);
    if (out < 0) {
        errno = -out;
        return -1;
    }

    return out;
}

int mkdir(const char* name, mode_t mode) {
    long out = syscall_mkdir(name, mode);
    if (out < 0) {
        errno = -out;
        return -1;
    }

    return out;
}

int stat(const char* restrict file, struct stat* restrict stat) {
    long out = syscall_statf(file, stat);
    if (out < 0) {
        errno = -out;
        return -1;
    }

    return out;
}

mode_t umask(mode_t mode) {
    return syscall_umask(mode);
}