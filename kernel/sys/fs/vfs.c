#include "vfs.h"

#include <sys/process.h>
#include <lib/alloc.h>
#include <lib/string.h>
#include <lib/kprintf.h>

fs_node_t* fs_root = 0;

size_t read_fs(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer) {
    if (node->read != 0) {
        return node->read(node, offset, size, buffer);
    } else {
        return 0;
    }
}

size_t write_fs(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer) {
    if (node->write != 0) {
        return node->write(node, offset, size, buffer);
    } else {
        return 0;
    }
}

void open_fs(fs_node_t* node, uint32_t flags) {
    if (node->open) {
        node->open(node, flags);
    }
}

void close_fs(fs_node_t* node) {
    if (node->close) {
        node->close(node);
    }
}

struct dirent* readdir_fs(fs_node_t* node, size_t index) {
    if ((node->flags & 0x07) == FS_DIRECTORY && node->readdir != 0) {
        return node->readdir(node, index);
    } else {
        return 0;
    }
}

fs_node_t* finddir_fs(fs_node_t* node, char* name) {
    if (node->finddir != 0) {
        return node->finddir(node, name);
    } else {
        return 0;
    }
}

fs_node_t* _kopen(const char* filename, unsigned int flags, fs_node_t* root, fs_node_t* wd) {
    if (!filename) {
        return 0;
    }

    if (!root) {
        kprintf("kopen: root is not mounted\n");
        return 0;
    }

    char* name = strdup(filename);
    char* nodename = name;

    if (*name == '/') {
        if (wd && wd != root) {
            free(wd);
        }
        wd = root;
        ++nodename;
    }

    if (!*nodename) {
        return wd;
    }

    if (!wd) {
        kprintf("kopen: no working directory\n");
        free(name);
        return 0;
    }

    for (char* p = nodename; 1; ++p) {
        if (*p == '/' || !*p) {
            char token = *p;
            *p++ = 0;
            fs_node_t* node = finddir_fs(wd, nodename);
            if (!*nodename) {
                node = wd;
            } else {
                if (wd != root) {
                    free(wd);
                }
                if (!node) {
                    free(name);
                    return 0;
                }
            }
            wd = node;
            if (wd->ptr) {
                fs_node_t* mount = wd->ptr;
                free(wd);
                wd = mount;
            }
            open_fs(wd, flags);
            if (!token) {
                free(name);
                return wd;
            }

            nodename = p;
        }
    }
}

fs_node_t* kopen(const char* filename, unsigned int flags) {
    return _kopen(filename, flags, fs_root, _kopen(current_process->pwd, flags, fs_root, fs_root));
}