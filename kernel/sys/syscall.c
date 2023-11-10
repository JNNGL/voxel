#include "syscall.h"

#include <cpu/mmu.h>
#include <sys/errno.h>
#include <sys/process.h>
#include <lib/alloc.h>
#include <lib/string.h>
#include <lib/elf.h>
#include <stdint.h>

static int validate_ptr(void* ptr) {
    if (!ptr) {
        return 0;
    }

    if ((uintptr_t) ptr <= current_process->image.entry
        || (uintptr_t) ptr >= 0x8000000000000000) {
        return 1;
    }

    if (!mmu_validate_user(ptr, 1, 0)) {
        return 1;
    }
}

long sys_exit(long code) {
    process_exit((code & 0xFF) << 8);
    __builtin_unreachable();
}

long sys_fork() {
    return fork();
}

long sys_read(long fd, char* ptr, uintptr_t len) {
    fs_node_t* node = get_process_fd(fd);
    if (!node) {
        return -EBADF;
    }

    if (!mmu_validate_user(ptr, len, MMU_PTR_NULL | MMU_PTR_WRITE)) {
        return -EINVAL;
    }

    if (len && !ptr) {
        return -EFAULT;
    }

    if (!(node->user_mode & 01)) {
        return -EACCES;
    }

    long out = read_fs(node, node->user_offset, len, (uint8_t*) ptr);
    if (out > 0) {
        node->user_offset += out;
    }

    return out;
}

long sys_write(long fd, char* ptr, uintptr_t len) {
    fs_node_t* node = get_process_fd(fd);
    if (!node) {
        return -EBADF;
    }

    if (!mmu_validate_user(ptr, len, MMU_PTR_NULL)) {
        return -EINVAL;
    }

    if (len && !ptr) {
        return -EFAULT;
    }

    if (!(node->user_mode & 02)) {
        return -EACCES;
    }

    long out = write_fs(node, node->user_offset, len, (uint8_t*) ptr);
    if (out > 0) {
        node->user_offset += out;
    }

    return out;
}

long sys_open(const char* file, long flags, long mode) {
    validate_ptr((void*) file);
    if (!file) {
        return -EFAULT;
    }

    fs_node_t* node = kopen(file, flags);

    int access_bit = 0;
    if (node && (flags & O_CREAT) && (flags & O_EXCL)) {
        close_fs(node);
        free(node);
        return -EEXIST;
    }

    if (!(flags & O_WRONLY) || (flags & O_RDWR)) {
        if (node && !check_permission(node, 04)) {
            close_fs(node);
            free(node);
            return -EACCES;
        } else {
            access_bit |= 01;
        }
    }

    if ((flags & O_RDWR) || (flags & O_WRONLY)) {
        if (node && !check_permission(node, 02)) {
            close_fs(node);
            free(node);
            return -EACCES;
        }
        if (node && (node->flags & FS_DIRECTORY)) {
            free(node);

            return -EISDIR;
        }
        if ((flags & O_RDWR) || (flags & O_WRONLY)) {
            access_bit |= 02;
        }
    }

    if (!node && (flags & O_CREAT)) {
        int result = create_file((char*) file, mode);
        if (!result) {
            node = kopen(file, flags);
        } else {
            return result;
        }
    }

    if (node && (flags & O_DIRECTORY)) {
        if (!(node->flags & FS_DIRECTORY)) {
            free(node);
            return -ENOTDIR;
        }
    }

    if (node && (flags & O_TRUNC)) {
        if (!(access_bit & 02)) {
            close_fs(node);
            free(node);
            return -EINVAL;
        }

        truncate_fs(node);
    }

    if (!node) {
        return -ENOTDIR;
    }

    if (node && (flags & O_CREAT) && (node->flags & FS_DIRECTORY)) {
        close_fs(node);
        free(node);
        return -EISDIR;
    }

    int fd = append_process_fd(node);
    node->user_mode = access_bit;
    if (flags & O_APPEND) {
        node->user_offset = node->length;
    } else {
        node->user_offset = 0;
    }

    return fd;
}

long sys_close(long fd) {
    fs_node_t* node = get_process_fd(fd);
    if (!node) {
        return -EBADF;
    }

    close_fs(node);
    current_process->fds[fd] = 0;
    free(node);
    return 0;
}

long sys_seek(long fd, long offset, long whence) {
    fs_node_t* node = get_process_fd(fd);
    if (!node) {
        return -EBADF;
    }

    if (node->flags & (FS_PIPE | FS_CHARDEVICE)) {
        return -ESPIPE;
    }

    switch (whence) {
        case SEEK_SET:
            node->user_offset = offset;
            break;

        case SEEK_CUR:
            node->user_offset += offset;
            break;

        case SEEK_END:
            node->user_offset = node->length + offset;
            break;

        default:
            return -EINVAL;
    }

    return node->user_offset;
}

long sys_getpid() {
    return current_process->pid;
}

long sys_sbrk(long size) {
    if (size & 0xFFF) {
        return -EINVAL;
    }

    volatile process_t* process = current_process;
    uintptr_t out = process->image.heap;
    for (uintptr_t i = out; i < out + size; i += 0x1000) {
        pagemap_entry_t* page = mmu_lookup_frame(i, MMU_GET_MAKE);
        mmu_allocate_frame_ex(page, MMU_FLAG_WRITABLE);
    }
    process->image.heap += size;
    return out;
}

long sys_uname(struct {
    char sysname[256];
    char nodename[256];
    char release[256];
    char version[256];
    char machine[256];
}* name) {
    validate_ptr((void*) name);
    if (!name) {
        return -EFAULT;
    }

    strcpy(name->sysname, "voxel");
    strcpy(name->nodename, "localhost");
    strcpy(name->release, "\"Unsought Origin\"");
    strcpy(name->version, "Voxel 1.0.0-dev");
    strcpy(name->machine, "x86_64");
    return 0;
}

static long _stat(fs_node_t* node, struct {
    int st_dev;
    int st_ino;
    int st_mode;
    uint16_t st_nlink;
    int st_uid;
    int st_gid;
    int st_rdev;
    long st_size;
    long st_atim;
    long st_mtim;
    long st_ctim;
    unsigned long st_blksize;
    unsigned long st_blocks;
}* st) {
    if (!node) {
        return -ENOENT;
    }

    st->st_dev = (uint16_t) (((uintptr_t) node->device & 0xFFFF0) >> 8);
    st->st_ino = node->inode;

    uint32_t flags;
    if (node->flags & FS_FILE) {
        flags |= S_IFREG;
    }
    if (node->flags & FS_DIRECTORY) {
        flags |= S_IFDIR;
    }
    if (node->flags & FS_CHARDEVICE) {
        flags |= S_IFCHR;
    }
    if (node->flags & FS_BLOCKDEVICE) {
        flags |= S_IFIFO;
    }
    if (node->flags & FS_SYMLINK) {
        flags |= S_IFLNK;
    }

    st->st_mode = node->mask | flags;
    st->st_nlink = node->nlink;
    st->st_uid = node->uid;
    st->st_gid = node->gid;
    st->st_rdev = 0;
    st->st_size = node->length;
    st->st_atim = 0; // TODO
    st->st_mtim = 0;
    st->st_ctim = 0;
    st->st_blksize = 512;
    return 0;
}

long sys_stat(long fd, void* ptr) {
    fs_node_t* node = get_process_fd(fd);
    if (!node) {
        return -EBADF;
    }

    validate_ptr(ptr);
    if (!ptr) {
        return -EFAULT;
    }

    return _stat(node, ptr);
}

long sys_statf(char* path, void* ptr) {
    validate_ptr(path);
    validate_ptr(ptr);

    if (!path || !ptr) {
        return -EFAULT;
    }

    fs_node_t* node = kopen(path, 0);
    long r = _stat(node, ptr);
    if (node) {
        close_fs(node);
    }

    return r;
}

long sys_lstat(char* path, void* ptr) {
    validate_ptr(path);
    validate_ptr(ptr);

    if (!path || !ptr) {
        return -EFAULT;
    }

    fs_node_t* node = kopen(path, 0); // TODO: O_PATH | O_NOFOLLOW
    long r = _stat(node, ptr);
    if (node) {
        close_fs(node);
    }

    return r;
}

long sys_readdir(long fd, long index, struct dirent* ptr) {
    fs_node_t* node = get_process_fd(fd);
    if (!node) {
        return -EBADF;
    }

    validate_ptr(ptr);
    if (!ptr) {
        return -EFAULT;
    }

    struct dirent* dirent = readdir_fs(node, (size_t) index);
    if (dirent) {
        memcpy(ptr, dirent, sizeof(struct dirent));
        free(dirent);
        return 1;
    } else {
        return 0;
    }
}

long sys_mkdir(char* path, long mode) {
    validate_ptr(path);
    if (!path) {
        return -EFAULT;
    }

    return mkdir(path, (int) mode);
}

long sys_ioctl(long fd, unsigned long cmd, void* arg) {
    fs_node_t* node = get_process_fd(fd);
    if (!node) {
        return -EBADF;
    }

    validate_ptr(arg);
    return ioctl_fs(node, cmd, arg);
}

long sys_access(char* path, long flags) {
    validate_ptr(path);
    if (!path) {
        return -EFAULT;
    }

    fs_node_t* node = kopen(path, 0);
    if (!node) {
        return -ENOENT;
    }

    close_fs(node);
    free(node);
    return 0;
}

long sys_chmod(char* path, long mode) {
    validate_ptr(path);
    if (!path) {
        return -EFAULT;
    }

    fs_node_t* node = kopen(path, 0);
    if (!node) {
        return -ENOENT;
    }

    if (current_process->uid != 0 && current_process->uid != node->uid) {
        close_fs(node);
        free(node);
        return -EACCES;
    }

    long r = chmod_fs(node, mode);
    close_fs(node);
    free(node);
    return r;
}

long sys_umask(long mode) {
    current_process->mask = mode & 0777;
    return 0;
}

long sys_chown(char* path, long uid, long gid) {
    validate_ptr(path);
    if (!path) {
        return -EFAULT;
    }

    fs_node_t* node = kopen(path, 0);
    if (!node) {
        return -ENOENT;
    }

    if (current_process->uid != 0 && uid != -1) {
        close_fs(node);
        free(node);
        return -EACCES;
    }

    if (current_process->uid != 0 && gid != -1) {
        if (current_process->uid != node->uid
            || current_process->gid != node->gid) {
            close_fs(node);
            free(node);
            return -EACCES;
        }
    }

    // TODO: Clear SUID

    long r = chown_fs(node, uid, gid);
    close_fs(node);
    free(node);
    return r;
}

long sys_unlink(char* path) {
    validate_ptr(path);
    if (!path) {
        return -EFAULT;
    }

    return unlink(path);
}

long sys_symlink(char* target, char* name) {
    validate_ptr(target);
    validate_ptr(name);
    if (!target || !name) {
        return -EFAULT;
    }

    return symlink(target, name);
}

long sys_readlink(char* path, char* ptr, long len) {
    validate_ptr(path);
    if (!mmu_validate_user(ptr, len, 0)) {
        return -EFAULT;
    }

    fs_node_t* node = kopen(path, 0); // TODO: O_PATH | O_NOFOLLOW
    if (!node) {
        return -ENOENT;
    }

    long r = readlink_fs(node, ptr, len);
    close_fs(node);
    free(node);
    return r;
}

long sys_dup2(long from, long to) {
    return process_dup2(from, to);
}

long sys_execve(const char* file, char* const argv[], char* const envp[]) {
    validate_ptr((void*) file);
    validate_ptr((void*) argv);
    validate_ptr((void*) envp);

    if (!file || !argv) {
        return -EFAULT;
    }

    fs_node_t* node = kopen(file, 0);
    if (!node) {
        return -ENOENT;
    }

    int argc = 0;
    int envc = 0;
    while (argv[argc]) {
        validate_ptr(argv[argc++]);
    }

    if (envp) {
        while (envp[envc]) {
            validate_ptr(envp[envc++]);
        }
    }

    char** kargv = malloc(sizeof(char*) * (argc + 1));
    for (int i = 0; i < argc; ++i) {
        size_t len = strlen(argv[i]) + 1;
        kargv[i] = malloc(len);
        memcpy(kargv[i], argv[i], len);
    }
    kargv[argc] = 0;

    char** kenvp;
    if (envp && envc) {
        kenvp = malloc(sizeof(char*) * (envc + 1));
        for (int i = 0; i < envc; ++i) {
            size_t len = strlen(envp[i]) + 1;
            kenvp[i] = malloc(len);
            memcpy(kenvp[i], envp[i], len);
        }
        kenvp[envc] = 0;
    } else {
        kenvp = malloc(sizeof(char*));
        *kenvp = 0;
    }

    for (size_t i = 3; i < current_process->fd_length; ++i) {
        if (current_process->fds[i]) {
            close_fs(current_process->fds[i]);
            free(current_process->fds[i]);
            current_process->fds[i] = 0;
        }
    }

    if (current_process->fd_length > 3) {
        current_process->fd_length = 3;
    }

    return elf_exec(file, node, argc, (const char**) kargv, (const char**) kenvp);
}

long sys_waitpid(long pid) {
    process_t* proc = get_process(pid);

    if (!proc) {
        return 0;
    }

    while (!(proc->flags & PROC_FLAG_FINISHED)) {
        proc->flags |= PROC_FLAG_HAS_WAITERS;
        switch_task(1);
    }

    int ret = proc->status;
    process_delete(proc);
    return ret;
}

typedef uintptr_t(*syscall_func_t)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);

static syscall_func_t syscall_funcs[] = {
        [SYS_EXIT]     = (syscall_func_t) sys_exit,
        [SYS_FORK]     = (syscall_func_t) sys_fork,
        [SYS_READ]     = (syscall_func_t) sys_read,
        [SYS_WRITE]    = (syscall_func_t) sys_write,
        [SYS_OPEN]     = (syscall_func_t) sys_open,
        [SYS_CLOSE]    = (syscall_func_t) sys_close,
        [SYS_SEEK]     = (syscall_func_t) sys_seek,
        [SYS_GETPID]   = (syscall_func_t) sys_getpid,
        [SYS_SBRK]     = (syscall_func_t) sys_sbrk,
        [SYS_UNAME]    = (syscall_func_t) sys_uname,
        [SYS_STAT]     = (syscall_func_t) sys_stat,
        [SYS_STATF]    = (syscall_func_t) sys_statf,
        [SYS_LSTAT]    = (syscall_func_t) sys_lstat,
        [SYS_READDIR]  = (syscall_func_t) sys_readdir,
        [SYS_MKDIR]    = (syscall_func_t) sys_mkdir,
        [SYS_IOCTL]    = (syscall_func_t) sys_ioctl,
        [SYS_ACCESS]   = (syscall_func_t) sys_access,
        [SYS_CHMOD]    = (syscall_func_t) sys_chmod,
        [SYS_UMASK]    = (syscall_func_t) sys_umask,
        [SYS_CHOWN]    = (syscall_func_t) sys_chown,
        [SYS_UNLINK]   = (syscall_func_t) sys_unlink,
        [SYS_SYMLINK]  = (syscall_func_t) sys_symlink,
        [SYS_READLINK] = (syscall_func_t) sys_readlink,
        [SYS_DUP2]     = (syscall_func_t) sys_dup2,
        [SYS_EXECVE]   = (syscall_func_t) sys_execve,
        [SYS_WAITPID]  = (syscall_func_t) sys_waitpid,
};

void syscall_handler(struct regs* r) {
    current_process->syscall_regs = r;
    uintptr_t rval;

    if (r->rax >= sizeof(syscall_funcs) / sizeof(syscall_func_t)) {
        rval = -EINVAL;
        goto ret;
    }

    syscall_func_t func = syscall_funcs[r->rax];
    if (!func) {
        rval = -EINVAL;
        goto ret;
    }

    rval = func(r->rbx, r->rcx, r->rdx, r->rsi, r->rdi);
    if (rval == -ERESTART) {
        current_process->interrupted_syscall = r->rax;
    }

    ret:
    r->rax = rval;
}