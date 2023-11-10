#pragma once

#include <sys/fs/vfs.h>

struct tmpfs_file {
    char* name;
    int type;
    int mask;
    int uid;
    int gid;
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
    struct tmpfs_file** files;
    size_t files_capacity;
    size_t files_count;
    struct tmpfs_dir* parent;
};

fs_node_t* tmpfs_create(const char* name);