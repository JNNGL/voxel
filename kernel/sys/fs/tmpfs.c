#include "tmpfs.h"

#include <sys/process.h>
#include <sys/errno.h>
#include <lib/alloc.h>
#include <lib/string.h>
#include <lib/kprintf.h>
#include <cpu/mmu.h>

#define BLOCKSIZE 0x1000

#define TMPFS_TYPE_FILE 1
#define TMPFS_TYPE_DIR  2
#define TMPFS_TYPE_LINK 3

static struct tmpfs_dir* tmpfs_root;
static volatile intptr_t tmpfs_total_blocks = 0;

static fs_node_t* tmpfs_from_dir(struct tmpfs_dir* dir);

static struct tmpfs_file* tmpfs_new_file(char* name) {
    struct tmpfs_file* f = malloc(sizeof(struct tmpfs_file));
    f->name = strdup(name);
    f->type = TMPFS_TYPE_FILE;
    f->length = 0;
    f->pointers = 2;
    f->block_count = 0;
    f->mask = 0;
    f->uid = 0;
    f->gid = 0;
    f->blocks = malloc(f->pointers * sizeof(uintptr_t*));
    for (size_t i = 0; i < f->pointers; ++i) {
        f->blocks[i] = 0;
    }

    return f;
}

static int symlink_tmpfs(fs_node_t* parent, char* target, char* name) {
    struct tmpfs_dir* d = (struct tmpfs_dir*) parent->device;

    for (size_t i = 0; i < d->files_count; i++) {
        struct tmpfs_file* f = d->files[i];
        if (!strcmp(name, f->name)) {
            return -EEXIST;
        }
    }

    struct tmpfs_file* f = tmpfs_new_file(name);
    f->type = TMPFS_TYPE_LINK;
    f->target = strdup(target);
    f->length = strlen(target);
    f->mask = 0777;
    f->uid = current_process->uid;
    f->gid = current_process->gid;
    if (d->files_count >= d->files_capacity) {
        d->files_capacity += 8;
        d->files = realloc(d->files, sizeof(struct tmpfs_file*) * d->files_capacity);
    }
    d->files[d->files_count++] = f;
    return 0;
}

static size_t readlink_tmpfs(fs_node_t* node, char* buf, size_t size) {
    struct tmpfs_file* f = (struct tmpfs_file*) node->device;

    if (f->type != TMPFS_TYPE_LINK) {
        kprintf("tmpfs: not a symlink\n");
        return -1;
    }

    if (size < strlen(f->target) + 1) {
        memcpy(buf, f->target, size - 1);
        buf[size - 1] = 0;
        return size - 2;
    } else {
        size_t len = strlen(f->target);
        memcpy(buf, f->target, len + 1);
        return len;
    }
}

static struct tmpfs_dir* tmpfs_new_dir(const char* name, struct tmpfs_dir* parent) {
    struct tmpfs_dir* d = malloc(sizeof(struct tmpfs_dir));
    d->name = strdup(name);
    d->type = TMPFS_TYPE_DIR;
    d->mask = 0;
    d->uid = 0;
    d->gid = 0;
    d->parent = parent;
    d->files_capacity = 8;
    d->files_count = 0;
    d->files = malloc(sizeof(struct tmpfs_file*) * d->files_capacity);
    return d;
}

static void tmpfs_free_file(struct tmpfs_file* f) {
    if (f->type == TMPFS_TYPE_LINK) {
        free(f->target);
    }

    for (size_t i = 0; i < f->block_count; ++i) {
        mmu_release_frame((uintptr_t) f->blocks[i] * 0x1000);
        --tmpfs_total_blocks;
    }
}

static void tmpfs_file_blocks_embiggen(struct tmpfs_file* f) {
    f->pointers *= 2;
    f->blocks = realloc(f->blocks, sizeof(uintptr_t*) * f->pointers);
}

static char* tmpfs_file_getset_block(struct tmpfs_file* f, size_t blockid, int create) {
    if (create) {
        while (blockid >= f->pointers) {
            tmpfs_file_blocks_embiggen(f);
        }

        while (blockid >= f->block_count) {
            uintptr_t index = mmu_allocate_frame();
            ++tmpfs_total_blocks;
            f->blocks[f->block_count] = index;
            ++f->block_count;
        }
    } else {
        if (blockid >= f->block_count) {
            kprintf("tmpfs: not enough blocks\n");
            return 0;
        }
    }

    return (char*) mmu_from_physical(f->blocks[blockid] << 12);
}

static size_t read_tmpfs(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer) {
    struct tmpfs_file* f = (struct tmpfs_file*) node->device;

    size_t end;
    if (offset + size > f->length) {
        end = f->length;
    } else {
        end = offset + size;
    }

    uint64_t start_block = offset / BLOCKSIZE;
    uint64_t end_block = end / BLOCKSIZE;
    uint64_t end_size = end - end_block * BLOCKSIZE;
    uint64_t size_to_read = end - offset;
    if (start_block == end_block && offset == end) {
        return 0;
    }
    if (start_block == end_block) {
        void* buf = tmpfs_file_getset_block(f, start_block, 0);
        memcpy(buffer, (uint8_t*) (((uintptr_t) buf) + ((uintptr_t) offset % BLOCKSIZE)), size_to_read);
        return size_to_read;
    } else {
        uint64_t block_offset;
        uint64_t blocks_read = 0;
        for (block_offset = start_block; block_offset < end_block; block_offset++, blocks_read++) {
            if (block_offset == start_block) {
                void* buf = tmpfs_file_getset_block(f, block_offset, 0);
                memcpy(buffer, (uint8_t*) (((uint64_t) buf) + ((uintptr_t) offset % BLOCKSIZE)), BLOCKSIZE - (offset % BLOCKSIZE));
            } else {
                void* buf = tmpfs_file_getset_block(f, block_offset, 0);
                memcpy(buffer + BLOCKSIZE * blocks_read - (offset % BLOCKSIZE), buf, BLOCKSIZE);
            }
        }

        if (end_size) {
            void* buf = tmpfs_file_getset_block(f, end_block, 0);
            memcpy(buffer + BLOCKSIZE * blocks_read - (offset % BLOCKSIZE), buf, end_size);
        }
    }

    return size_to_read;
}

static size_t write_tmpfs(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer) {
    struct tmpfs_file* f = (struct tmpfs_file*) node->device;

    uint64_t end;
    if (offset + size > f->length) {
        f->length = offset + size;
    }
    end = offset + size;
    uint64_t start_block = offset / BLOCKSIZE;
    uint64_t end_block = end / BLOCKSIZE;
    uint64_t end_size = end - end_block * BLOCKSIZE;
    uint64_t size_to_read = end - offset;
    if (start_block == end_block) {
        void* buf = tmpfs_file_getset_block(f, start_block, 1);
        memcpy((uint8_t*) (((uint64_t) buf) + ((uintptr_t) offset % BLOCKSIZE)), buffer, size_to_read);
        return size_to_read;
    } else {
        uint64_t block_offset;
        uint64_t blocks_read = 0;
        for (block_offset = start_block; block_offset < end_block; block_offset++, blocks_read++) {
            if (block_offset == start_block) {
                void* buf = tmpfs_file_getset_block(f, block_offset, 1);
                memcpy((uint8_t*) (((uint64_t) buf) + ((uintptr_t) offset % BLOCKSIZE)), buffer, BLOCKSIZE - (offset % BLOCKSIZE));
            } else {
                void* buf = tmpfs_file_getset_block(f, block_offset, 1);
                memcpy(buf, buffer + BLOCKSIZE * blocks_read - (offset % BLOCKSIZE), BLOCKSIZE);
            }
        }

        if (end_size) {
            void* buf = tmpfs_file_getset_block(f, end_block, 1);
            memcpy(buf, buffer + BLOCKSIZE * blocks_read - (offset % BLOCKSIZE), end_size);
        }
    }

    return size_to_read;
}

static int chmod_tmpfs(fs_node_t* node, int mode) {
    struct tmpfs_file* f = (struct tmpfs_file*) node->device;
    f->mask = mode;
    return 0;
}

static int chown_tmpfs(fs_node_t* node, int uid, int gid) {
    struct tmpfs_file* f = (struct tmpfs_file*) node->device;

    if (uid != -1) {
        f->uid = uid;
    }

    if (gid != -1) {
        f->gid = gid;
    }

    return 0;
}

static int truncate_tmpfs(fs_node_t* node) {
    struct tmpfs_file* f = (struct tmpfs_file*) node->device;
    for (size_t i = 0; i < f->block_count; ++i) {
        mmu_release_frame((uintptr_t) f->blocks[i] * 0x1000);
        --tmpfs_total_blocks;
        f->blocks[i] = 0;
    }

    f->block_count = 0;
    f->length = 0;
    return 0;
}

static void open_tmpfs(fs_node_t* node, unsigned int flags) {
    // set accessed time
}

static fs_node_t* tmpfs_from_file(struct tmpfs_file* f) {
    fs_node_t* fnode = malloc(sizeof(fs_node_t));
    memset(fnode, 0, sizeof(fs_node_t));
    fnode->inode = 0;
    strcpy(fnode->name, f->name);
    fnode->device = f;
    fnode->mask = f->mask;
    fnode->uid = f->uid;
    fnode->gid = f->gid;
    fnode->flags = FS_FILE;
    fnode->read = read_tmpfs;
    fnode->write = write_tmpfs;
    fnode->open = open_tmpfs;
    fnode->close = 0;
    fnode->readdir = 0;
    fnode->finddir = 0;
    fnode->chmod = chmod_tmpfs;
    fnode->chown = chown_tmpfs;
    fnode->length = f->length;
    fnode->truncate = truncate_tmpfs;
    fnode->nlink = 1;
    return fnode;
}

static fs_node_t* tmpfs_from_link(struct tmpfs_file* f) {
    fs_node_t* fnode = tmpfs_from_file(f);
    fnode->flags |= FS_SYMLINK;
    fnode->readlink = readlink_tmpfs;
    fnode->read = 0;
    fnode->write = 0;
    fnode->create = 0;
    fnode->mkdir = 0;
    fnode->readdir = 0;
    fnode->finddir = 0;
    return fnode;
}

static struct dirent* readdir_tmpfs(fs_node_t* node, size_t index) {
    struct tmpfs_dir* d = (struct tmpfs_dir*) node->device;
    if (d != tmpfs_root) {
        if (index == 0) {
            struct dirent *out = malloc(sizeof(struct dirent));
            memset(out, 0, sizeof(struct dirent));
            out->ino = 0;
            strcpy(out->name, ".");
            return out;
        }

        if (index == 1) {
            struct dirent *out = malloc(sizeof(struct dirent));
            memset(out, 0, sizeof(struct dirent));
            out->ino = 0;
            strcpy(out->name, "..");
            return out;
        }

        index -= 2;
    }

    if (index >= d->files_count) {
        return 0;
    }

    struct tmpfs_file* f = d->files[index];
    if (!f) {
        return 0;
    }

    struct dirent* dirent = malloc(sizeof(struct dirent));
    strcpy(dirent->name, f->name);
    dirent->ino = (uintptr_t) f;
    return dirent;
}

static fs_node_t* finddir_tmpfs(fs_node_t* node, char* name) {
    if (!name) {
        return 0;
    }

    struct tmpfs_dir* d = (struct tmpfs_dir*) node->device;

    for (size_t i = 0; i < d->files_count; i++) {
        struct tmpfs_file* f = d->files[i];
        if (!strcmp(name, f->name)) {
            switch (f->type) {
                case TMPFS_TYPE_FILE: return tmpfs_from_file(f);
                case TMPFS_TYPE_LINK: return tmpfs_from_link(f);
                case TMPFS_TYPE_DIR: return tmpfs_from_dir((struct tmpfs_dir*) f);
                default: return 0;
            }
        }
    }

    return 0;
}

static int unlink_tmpfs(fs_node_t* node, char* name) {
    struct tmpfs_dir* d = (struct tmpfs_dir*) node->device;

    for (size_t i = 0; i < d->files_count; i++) {
        struct tmpfs_file* f = d->files[i];
        if (!strcmp(name, f->name)) {
            if (f->type == TMPFS_TYPE_DIR) {
                if (((struct tmpfs_dir*) f)->files) {
                    return -ENOTEMPTY;
                }
            } else {
                tmpfs_free_file(f);
            }

            --d->files_count;
            size_t entries_after = d->files_count - i;
            if (entries_after) {
                memmove(&d->files[i], &d->files[i + 1], sizeof(*d->files) * entries_after);
            }

            free(f->name);
            free(f);
            return 0;
        }
    }

    return -ENOENT;
}

static int create_tmpfs(fs_node_t* parent, char* name, int mode) {
    if (!name) {
        return -EINVAL;
    }

    struct tmpfs_dir* d = (struct tmpfs_dir*) parent->device;

    for (size_t i = 0; i < d->files_count; i++) {
        struct tmpfs_file* f = d->files[i];
        if (!strcmp(name, f->name)) {
            return -EEXIST;
        }
    }

    struct tmpfs_file* f = tmpfs_new_file(name);
    f->mask = mode;
    f->uid = current_process->uid;
    f->gid = current_process->gid;
    if (d->files_count >= d->files_capacity) {
        d->files_capacity += 8;
        d->files = realloc(d->files, sizeof(struct tmpfs_file*) * d->files_capacity);
    }
    d->files[d->files_count++] = f;
    return 0;
}

static int mkdir_tmpfs(fs_node_t* parent, char* name, int mode) {
    if (!name) {
        return -EINVAL;
    }

    if (!strlen(name)) {
        return -EINVAL;
    }

    struct tmpfs_dir* d = (struct tmpfs_dir*) parent->device;

    for (size_t i = 0; i < d->files_count; i++) {
        struct tmpfs_file* f = d->files[i];
        if (!strcmp(name, f->name)) {
            return -EEXIST;
        }
    }

    struct tmpfs_dir* out = tmpfs_new_dir(name, d);
    out->mask = mode;
    out->uid = current_process->uid;
    out->gid = current_process->gid;
    if (d->files_count >= d->files_capacity) {
        d->files_capacity += 8;
        d->files = realloc(d->files, sizeof(struct tmpfs_file*) * d->files_capacity);
    }
    d->files[d->files_count++] = (struct tmpfs_file*) out;
    return 0;
}

static fs_node_t* tmpfs_from_dir(struct tmpfs_dir* d) {
    fs_node_t* fnode = malloc(sizeof(fs_node_t));
    memset(fnode, 0, sizeof(fs_node_t));
    fnode->inode = 0;
    strcpy(fnode->name, d->name);
    fnode->mask = d->mask;
    fnode->uid = d->uid;
    fnode->gid = d->gid;
    fnode->device = d;
    fnode->flags = FS_DIRECTORY;
    fnode->read = 0;
    fnode->write = 0;
    fnode->open = 0;
    fnode->close = 0;
    fnode->readdir = readdir_tmpfs;
    fnode->finddir = finddir_tmpfs;
    fnode->create = create_tmpfs;
    fnode->unlink = unlink_tmpfs;
    fnode->mkdir = mkdir_tmpfs;
    fnode->symlink = symlink_tmpfs;
    fnode->chown = chown_tmpfs;
    fnode->chmod = chmod_tmpfs;
    fnode->nlink = 1;
    return fnode;
}

fs_node_t* tmpfs_create(const char* name) {
    tmpfs_root = tmpfs_new_dir(name, 0);
    tmpfs_root->mask = 0777;
    tmpfs_root->uid = 0;
    tmpfs_root->gid = 0;
    return tmpfs_from_dir(tmpfs_root);
}