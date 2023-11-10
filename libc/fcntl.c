#include <sys/syscall.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>

int creat(const char* path, mode_t mode) {
    return open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);
}

int open(const char* path, int flags, ...) {
    va_list list;
    int mode = 0;
    va_start(list, flags);
    if (flags & O_CREAT) {
        mode = va_arg(list, int);
    }
    va_end(list);

    long out = syscall_open(path, flags, mode);
    if (out == -1) {
        if (flags & O_CREAT) {
            errno = EACCES;
        } else {
            errno = ENOENT;
        }
    } else if (out < 0) {
        errno = -out;
        return -1;
    }

    return out;
}