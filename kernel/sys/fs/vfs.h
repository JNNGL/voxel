#pragma once

#include <sys/fs/dirent.h>
#include <stdint.h>
#include <stddef.h>

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE        0x05
#define FS_SYMLINK     0x06
#define FS_MOUNTPOINT  0x08

#define OPEN_FLAG_PARENT 0x10000000

struct fs_node;

typedef size_t(*read_type_t)(struct fs_node*, size_t, size_t, uint8_t*);
typedef size_t(*write_type_t)(struct fs_node*, size_t, size_t, uint8_t*);
typedef void(*open_type_t)(struct fs_node*, uint32_t);
typedef void(*close_type_t)(struct fs_node*);
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
    struct fs_node* ptr;
} fs_node_t;

extern fs_node_t* fs_root;

size_t read_fs(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer);
size_t write_fs(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer);
void open_fs(fs_node_t* node, uint32_t flags);
void close_fs(fs_node_t* node);
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

int create_file(char* name, int mode);
int mkdir(char* name, int mode);
int symlink(char* target, char* name);
int unlink(char* name);

fs_node_t* kopen(const char* filename, unsigned int flags);