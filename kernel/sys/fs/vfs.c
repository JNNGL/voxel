#include "vfs.h"

#include <sys/process.h>
#include <sys/errno.h>
#include <lib/alloc.h>
#include <lib/string.h>
#include <lib/kprintf.h>

fs_node_t* fs_root = 0;

int check_permission(fs_node_t* node, int perm) {
    if (!node) {
        return 0;
    }

    if (current_process->uid == 0 && perm != 01) {
        return 1;
    }

    uint8_t permissions = node->mask & 07;
    uint8_t group = (node->mask >> 3) & 07;
    uint8_t user = (node->mask >> 6) & 07;

    if (current_process->uid == node->uid) {
        permissions |= user;
    }

    if (current_process->gid == node->gid) {
        permissions |= group;
    }

    return perm & permissions;
}

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

int truncate_fs(fs_node_t* node) {
    if (node->truncate) {
        return node->truncate(node);
    } else {
        return 0;
    }
}

int create_fs(fs_node_t* node, char* name, int mode) {
    if (node->create) {
        return node->create(node, name, mode);
    } else {
        return 0;
    }
}

int mkdir_fs(fs_node_t* node, char* name, int mode) {
    if (node->mkdir) {
        return node->mkdir(node, name, mode);
    } else {
        return 0;
    }
}

int symlink_fs(fs_node_t* node, char* target, char* name) {
    if (node->symlink) {
        return node->symlink(node, target, name);
    } else {
        return 0;
    }
}

int unlink_fs(fs_node_t* node, char* name) {
    if (node->unlink) {
        return node->unlink(node, name);
    } else {
        return 0;
    }
}

size_t readlink_fs(fs_node_t* node, char* buf, size_t size) {
    if (node->readlink) {
        return node->readlink(node, buf, size);
    } else {
        return 0;
    }
}

int chmod_fs(fs_node_t* node, int mode) {
    if (node->chmod) {
        return node->chmod(node, mode);
    } else {
        return 0;
    }
}

int chown_fs(fs_node_t* node, int uid, int gid) {
    if (node->chown) {
        return node->chown(node, uid, gid);
    } else {
        return 0;
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

int create_file(char* name, int mode) {
    if (!name || !*name) {
        return -EINVAL;
    }

    if (!strcmp(name, "/")) {
        return -EEXIST;
    }

    fs_node_t* parent = kopen(name, OPEN_FLAG_PARENT);
    if (!parent) {
        return -ENOENT;
    }

    if (!check_permission(parent, 02)) {
        return -EACCES;
    }

    char* filename = name;
    for (char* p = name; *p; p++) {
        if (*p == '/' && *(p + 1) && *(p + 1) != '/') {
            filename = p + 1;
        }
    }

    filename = strdup(filename);
    for (char* p = filename + strlen(filename) - 1; *p == '/'; p--) {
        *p = 0;
    }

    fs_node_t* this = finddir_fs(parent, filename);
    if (this) {
        free(this);
        return -EEXIST;
    }

    int result;
    if (parent->create) {
        result = parent->create(parent, filename, mode);
    } else {
        result = -EROFS;
    }

    close_fs(parent);
    free(parent);
    free(filename);
    return result;
}

int mkdir(char* name, int mode) {
    if (!name || !*name) {
        return -EINVAL;
    }

    fs_node_t* parent = kopen(name, OPEN_FLAG_PARENT);
    if (!parent) {
        return -ENOENT;
    }

    if (!check_permission(parent, 02)) {
        return -EACCES;
    }

    char* filename = name;
    for (char* p = name; *p; p++) {
        if (*p == '/' && *(p + 1) && *(p + 1) != '/') {
            filename = p + 1;
        }
    }

    filename = strdup(filename);
    for (char* p = filename + strlen(filename) - 1; *p == '/'; p--) {
        *p = 0;
    }

    fs_node_t* this = finddir_fs(parent, filename);
    if (this) {
        free(this);
        return -EEXIST;
    }

    int result;
    if (parent->mkdir) {
        result = parent->mkdir(parent, filename, mode);
    } else {
        result = -EROFS;
    }

    close_fs(parent);
    free(parent);
    free(filename);
    return result;
}

int symlink(char* target, char* name) {
    if (!name || !*name) {
        return -EINVAL;
    }

    fs_node_t* parent = kopen(name, OPEN_FLAG_PARENT);
    if (!parent) {
        return -ENOENT;
    }

    if (!check_permission(parent, 02)) {
        return -EACCES;
    }

    char* filename = name;
    for (char* p = name; *p; p++) {
        if (*p == '/' && *(p + 1) && *(p + 1) != '/') {
            filename = p + 1;
        }
    }

    filename = strdup(filename);
    for (char* p = filename + strlen(filename) - 1; *p == '/'; p--) {
        *p = 0;
    }

    int result;
    if (parent->symlink) {
        result = parent->symlink(parent, target, filename);
    } else {
        result = -EINVAL;
    }

    close_fs(parent);
    free(parent);
    free(filename);
    return result;
}

int unlink(char* name) {
    if (!name || !*name) {
        return -EINVAL;
    }

    fs_node_t* parent = kopen(name, OPEN_FLAG_PARENT);
    if (!parent) {
        return -ENOENT;
    }

    if (!check_permission(parent, 02)) {
        return -EACCES;
    }

    char* filename = name;
    for (char* p = name; *p; p++) {
        if (*p == '/' && *(p + 1) && *(p + 1) != '/') {
            filename = p + 1;
        }
    }

    filename = strdup(filename);
    for (char* p = filename + strlen(filename) - 1; *p == '/'; p--) {
        *p = 0;
    }

    int result;
    if (parent->unlink) {
        result = parent->unlink(parent, filename);
    } else {
        result = -EINVAL;
    }

    close_fs(parent);
    free(parent);
    free(filename);
    return result;
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

    if (!wd) {
        kprintf("kopen: no working directory\n");
        free(name);
        return 0;
    }

    if (!*nodename) {
        free(name);
        return wd;
    }

    for (char* p = nodename + strlen(nodename) - 1; *p == '/'; p--) {
        *p = 0;
    }

    for (char* p = nodename; 1; ++p) {
        if (*p == '/' || !*p) {
            char token = *p;
            if (!token && flags & OPEN_FLAG_PARENT) {
                free(name);
                return wd;
            }
            *p++ = 0;
            fs_node_t* node = finddir_fs(wd, nodename);
            if (!*nodename) {
                if (node) {
                    free(node);
                }
                node = wd;
            } else {
                if (wd != root) {
                    close_fs(wd);
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
    return _kopen(filename, flags, fs_root, _kopen(current_process->pwd, flags & ~OPEN_FLAG_PARENT, fs_root, fs_root));
}