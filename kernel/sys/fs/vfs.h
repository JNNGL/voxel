#pragma once

#include <sys/fs/dirent.h>
#include <stdint.h>
#include <stddef.h>

#define IOCTL_TYPE  0xF000

#define IOCTL_TYPE_TTY 0x01

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x04
#define FS_BLOCKDEVICE 0x08
#define FS_PIPE        0x10
#define FS_SYMLINK     0x20
#define FS_MOUNTPOINT  0x40

#define O_RDONLY    0x0000
#define O_WRONLY    0x0001
#define O_RDWR      0x0002
#define O_APPEND    0x0008
#define O_CREAT     0x0200
#define O_TRUNC     0x0400
#define O_EXCL      0x0800
#define O_NOFOLLOW  0x1000
#define O_PATH      0x2000
#define O_NONBLOCK  0x4000
#define O_DIRECTORY 0x8000

#define S_IFMT   0170000
#define S_IFBLK  0060000
#define S_IFCHR  0020000
#define S_IFIFO  0010000
#define S_IFREG  0100000
#define S_IFDIR  0040000
#define S_IFLNK  0120000
#define S_IFSOCK 0140000

#define OPEN_FLAG_PARENT 0x10000000

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

struct fs_node;

typedef size_t(*read_type_t)(struct fs_node*, size_t, size_t, uint8_t*);
typedef size_t(*write_type_t)(struct fs_node*, size_t, size_t, uint8_t*);
typedef void(*open_type_t)(struct fs_node*, uint32_t);
typedef void(*close_type_t)(struct fs_node*);
typedef int(*ioctl_type_t)(struct fs_node*, size_t, void*);
typedef int(*truncate_type_t)(struct fs_node*);
typedef int(*create_type_t)(struct fs_node*, char*, int);
typedef int(*mkdir_type_t)(struct fs_node*, char*, int);
typedef int(*symlink_type_t)(struct fs_node*, char*, char*);
typedef int(*unlink_type_t)(struct fs_node*, char*);
typedef size_t(*readlink_type_t)(struct fs_node*, char*, size_t);
typedef int(*chmod_type_t)(struct fs_node*, int);
typedef int(*chown_type_t)(struct fs_node*, int, int);
typedef struct dirent*(*readdir_type_t)(struct fs_node*, size_t);
typedef struct fs_node*(*finddir_type_t)(struct fs_node*, char*);

typedef struct fs_node {
    char name[256];
    void* device;
    uint32_t mask;
    uint32_t uid;
    uint32_t gid;
    uint32_t flags;
    uint32_t inode;
    uint32_t length;
    uint64_t context;
    read_type_t read;
    write_type_t write;
    open_type_t open;
    ioctl_type_t ioctl;
    close_type_t close;
    truncate_type_t truncate;
    create_type_t create;
    mkdir_type_t mkdir;
    symlink_type_t symlink;
    unlink_type_t unlink;
    readlink_type_t readlink;
    chmod_type_t chmod;
    chown_type_t chown;
    readdir_type_t readdir;
    finddir_type_t finddir;
    uint32_t nlink;
    size_t user_offset;
    int user_mode;
    struct fs_node* ptr;
} fs_node_t;

extern fs_node_t* fs_root;

int check_permission(fs_node_t* node, int perm);

size_t read_fs(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer);
size_t write_fs(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer);
void open_fs(fs_node_t* node, uint32_t flags);
void close_fs(fs_node_t* node);
int ioctl_fs(fs_node_t* node, size_t cmd, void* arg);
int truncate_fs(fs_node_t* node);
int create_fs(fs_node_t* node, char* name, int mode);
int mkdir_fs(fs_node_t* node, char* name, int mode);
int symlink_fs(fs_node_t* node, char* target, char* name);
int unlink_fs(fs_node_t* node, char* name);
size_t readlink_fs(fs_node_t* node, char* buf, size_t size);
int chmod_fs(fs_node_t* node, int mode);
int chown_fs(fs_node_t* node, int uid, int gid);
struct dirent* readdir_fs(fs_node_t* node, size_t index);
fs_node_t* finddir_fs(fs_node_t* node, char* name);

void vfs_init();
fs_node_t* vfs_get_mountpoint(const char* path);
int vfs_mount(char* path, fs_node_t* target);

int create_file(char* name, int mode);
int mkdir(char* name, int mode);
int symlink(char* target, char* name);
int unlink(char* name);

fs_node_t* kopen(const char* filename, unsigned int flags);