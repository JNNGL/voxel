#include "zero.h"

#include <sys/fs/vfs.h>
#include <lib/string.h>
#include <lib/alloc.h>

static size_t read_null(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer) {
    return 0;
}

static size_t write_null(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer) {
    return size;
}

static void open_null(fs_node_t* node, unsigned int flags) {

}

static void close_null(fs_node_t* node) {

}

fs_node_t* create_null_device() {
    fs_node_t* fnode = malloc(sizeof(fs_node_t));
    memset(fnode, 0, sizeof(fs_node_t));
    fnode->inode = 0;
    strcpy(fnode->name, "null");
    fnode->uid = 0;
    fnode->gid = 0;
    fnode->mask = 0666;
    fnode->flags = FS_CHARDEVICE;
    fnode->read = read_null;
    fnode->write = write_null;
    fnode->open = open_null;
    fnode->close = close_null;
    return fnode;
}

static size_t read_zero(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer) {
    memset(buffer, 0, size);
    return size;
}

static size_t write_zero(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer) {
    return size;
}

static void open_zero(fs_node_t* node, unsigned int flags) {

}

static void close_zero(fs_node_t* node) {

}

fs_node_t* create_zero_device() {
    fs_node_t* fnode = malloc(sizeof(fs_node_t));
    memset(fnode, 0, sizeof(fs_node_t));
    fnode->inode = 0;
    strcpy(fnode->name, "zero");
    fnode->uid = 0;
    fnode->gid = 0;
    fnode->mask = 0666;
    fnode->flags = FS_CHARDEVICE;
    fnode->read = read_zero;
    fnode->write = write_zero;
    fnode->open = open_zero;
    fnode->close = close_zero;
    return fnode;
}

void zerofs_mount() {
    vfs_mount("/dev/null", create_null_device());
    vfs_mount("/dev/zero", create_zero_device());
}