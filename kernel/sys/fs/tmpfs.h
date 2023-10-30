#pragma once

#include <sys/fs/vfs.h>

struct tmpfs_file;

struct tmpfs_flist {
    struct tmpfs_file* file;
    struct tmpfs_flist* next;
};

struct tmpfs_file {
    char* name;
    int type;
    int mask;
    int uid;
    int gid;
    struct tmpfs_flist list_node;
    size_t length;
    size_t block_count;
    size_t pointers;
    uintptr_t* blocks;
    char* target;
};

struct tmpfs_dir {
    char* name;
    int type;
    int mask;
    int uid;
    int gid;
    struct tmpfs_flist list_node;
    struct tmpfs_flist* files;
    struct tmpfs_dir* parent;
};

fs_node_t* tmpfs_create(const char* name);