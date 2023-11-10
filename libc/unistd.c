#include <unistd.h>

#include <sys/syscall.h>
#include <errno.h>

int access(const char* path, int mode) {
    long out = syscall_access(path, mode);
    if (out < 0) {
        errno = ENOENT;
        return -1;
    }

    return out;
}

int chown(const char* path, uid_t owner, gid_t group) {
    long out = syscall_chown(path, owner, group);
    if (out < 0) {
        errno = -out;
        return -1;
    }

    return out;
}

int close(int fd) {
    return syscall_close(fd);
}

int dup(int fd) {
    return dup2(fd, -1);
}

int dup2(int oldfd, int newfd) {
    return syscall_dup2(oldfd, newfd);
}

int execv(const char* path, char* const argv[]) {
    return execve(path, argv, environ);
}

int execve(const char* path, char* const argv[], char* const envp[]) {
    long out = syscall_execve(path, argv, envp);
    if (out < 0) {
        errno = -out;
        return -1;
    }

    return out;
}

pid_t fork() {
    return syscall_fork();
}

pid_t getpid() {
    return syscall_getpid();
}

pid_t getppid() {
    errno = EPERM;
    return -1;
}

off_t lseek(int fd, off_t off, int whence) {
    long out = syscall_seek(fd, off, whence);
    if (out < 0) {
        errno = -out;
        return -1;
    }

    return out;
}

ssize_t read(int fd, void* ptr, size_t len) {
    long out = syscall_read(fd, ptr, len);
    if (out < 0) {
        errno = -out;
        return -1;
    }

    return out;
}

ssize_t readlink(const char* restrict name, char* restrict buf, size_t len) {
    long out = syscall_readlink(name, buf, len);
    if (out < 0) {
        errno = -out;
        return -1;
    }

    return out;
}

int rmdir(const char* name) {
    errno = EPERM; // TODO: Implement this
    return -1;
}

int symlink(const char* target, const char* name) {
    long out = syscall_symlink(target, name);
    if (out < 0) {
        errno = -out;
        return -1;
    }

    return out;
}

int unlink(const char* path) {
    long out = syscall_unlink(path);
    if (out < 0) {
        errno = -out;
        return -1;
    }

    return out;
}

ssize_t write(int fd, const void* ptr, size_t len) {
    long out = syscall_write(fd, (char*) ptr, len);
    if (out < 0) {
        errno = -out;
        return -1;
    }

    return out;
}