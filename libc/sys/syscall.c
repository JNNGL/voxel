#include <sys/syscall.h>

#define DO_SYSCALL0(a) { long r; asm volatile("int $0x80" : "=a"(r) : "a"(a)); return r; }
#define DO_SYSCALL1(a, b) { long r; asm volatile("int $0x80" : "=a"(r) : "a"(a), "b"(b)); return r; }
#define DO_SYSCALL2(a, b, c) { long r; asm volatile("int $0x80" : "=a"(r) : "a"(a), "b"(b), "c"(c)); return r; }
#define DO_SYSCALL3(a, b, c, d) { long r; asm volatile("int $0x80" : "=a"(r) : "a"(a), "b"(b), "c"(c), "d"(d)); return r; }
#define DO_SYSCALL4(a, b, c, d, e) { long r; asm volatile("int $0x80" : "=a"(r) : "a"(a), "b"(b), "c"(c), "d"(d), "S"(e)); return r; }
#define DO_SYSCALL5(a, b, c, d, e, f) { long r; asm volatile("int $0x80" : "=a"(r) : "a"(a), "b"(b), "c"(c), "d"(d), "S"(e), "D"(f)); return r; }

long syscall_exit(int code) DO_SYSCALL1(SYS_EXIT, code)
long syscall_fork() DO_SYSCALL0(SYS_FORK)
long syscall_read(int fd, char* buf, size_t size) DO_SYSCALL3(SYS_READ, fd, buf, size)
long syscall_write(int fd, char* buf, size_t size) DO_SYSCALL3(SYS_WRITE, fd, buf, size)
long syscall_open(const char* file, int flags, int mode) DO_SYSCALL3(SYS_OPEN, file, flags, mode)
long syscall_close(int fd) DO_SYSCALL1(SYS_CLOSE, fd)
long syscall_seek(int fd, long offset, long whence) DO_SYSCALL3(SYS_SEEK, fd, offset, whence)
long syscall_getpid() DO_SYSCALL0(SYS_GETPID)
long syscall_sbrk(ssize_t size) DO_SYSCALL1(SYS_SBRK, size)
long syscall_uname(void* name) DO_SYSCALL1(SYS_UNAME, name)
long syscall_stat(int fd, void* st) DO_SYSCALL2(SYS_STAT, fd, st)
long syscall_statf(const char* file, void* st) DO_SYSCALL2(SYS_STATF, file, st)
long syscall_lstat(const char* file, void* st) DO_SYSCALL2(SYS_LSTAT, file, st)
long syscall_readdir(int fd, size_t index, void* dirent) DO_SYSCALL3(SYS_READDIR, fd, index, dirent)
long syscall_mkdir(const char* path, mode_t mode) DO_SYSCALL2(SYS_MKDIR, path, mode)
long syscall_ioctl(int fd, long cmd, void* arg) DO_SYSCALL3(SYS_IOCTL, fd, cmd, arg)
long syscall_access(const char* file, int flags) DO_SYSCALL2(SYS_ACCESS, file, flags)
long syscall_chmod(const char* file, mode_t mode) DO_SYSCALL2(SYS_CHMOD, file, mode)
long syscall_umask(mode_t mode) DO_SYSCALL1(SYS_UMASK, mode)
long syscall_chown(const char* file, uid_t uid, gid_t gid) DO_SYSCALL3(SYS_CHOWN, file, uid, gid)
long syscall_unlink(const char* file) DO_SYSCALL1(SYS_UNLINK, file)
long syscall_symlink(const char* target, const char* name) DO_SYSCALL2(SYS_SYMLINK, target, name)
long syscall_readlink(const char* file, char* ptr, size_t len) DO_SYSCALL3(SYS_READLINK, file, ptr, len)
long syscall_dup2(long oldfd, long newfd) DO_SYSCALL2(SYS_DUP2, oldfd, newfd)
long syscall_execve(const char* file, char* const argv[], char* const envp[]) DO_SYSCALL3(SYS_EXECVE, file, argv, envp)
long syscall_waitpid(pid_t pid) DO_SYSCALL1(SYS_WAITPID, pid)
