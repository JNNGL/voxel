#pragma once

#include <sys/fs/vfs.h>

fs_node_t* ramdiskfs_open(void* base);