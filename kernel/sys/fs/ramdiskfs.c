#include "ramdiskfs.h"

#include <stdint.h>
#include <lib/alloc.h>
#include <lib/string.h>

#define RDFS_SIGNATURE 0x1DF4

#define RDFS_TYPE_FILE      0
#define RDFS_TYPE_DIRECTORY 1
#define RDFS_TYPE_SYMLINK   2

typedef struct {
    uint16_t signature;
    uint8_t version;
    char label[16];
    uint32_t size;
    uint32_t root_entry;
} __attribute__((packed)) ramdiskfs_header_t;

typedef struct {
    uint8_t zero;
    char name[256];
    uint32_t offset;
    uint8_t type;
    uint8_t reserved;
    uint32_t next_entry;
    uint32_t target_entry;
    uint32_t size;
} __attribute__((packed)) ramdiskfs_entry_t;

static struct dirent dirent;
static fs_node_t rnode;

static uint32_t ramdiskfs_read(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    uintptr_t base = (uintptr_t) node->ptr->context;
    ramdiskfs_header_t* header = (ramdiskfs_header_t*) base;
    if (header->signature != RDFS_SIGNATURE) {
        return 0;
    }

    uint32_t current_entry = node->ptr == node ? header->root_entry : node->context;
    ramdiskfs_entry_t* entry = (ramdiskfs_entry_t*) (base + current_entry);

    if (offset > entry->size) {
        return 0;
    }

    if (offset + size > entry->size) {
        size = entry->size - offset;
    }

    memcpy(buffer, (void*) (base + entry->offset + sizeof(ramdiskfs_entry_t)), size);
    return size;
}

static struct dirent* ramdiskfs_readdir(fs_node_t* node, uint32_t index) {
    uintptr_t base = (uintptr_t) node->ptr->context;
    ramdiskfs_header_t* header = (ramdiskfs_header_t*) base;
    if (header->signature != RDFS_SIGNATURE) {
        return 0;
    }

    uint32_t current_entry = node->ptr == node ? header->root_entry : node->context;
    ramdiskfs_entry_t* entry = (ramdiskfs_entry_t*) (base + current_entry);
    if (!current_entry) {
        return 0;
    }

    current_entry = entry->target_entry;
    entry = (ramdiskfs_entry_t*) (base + current_entry);

    for (uint32_t i = 0; i < index; i++) {
        if (!current_entry) {
            return 0;
        }

        current_entry = entry->next_entry;
        entry = (ramdiskfs_entry_t*) (base + current_entry);
    }

    if (!current_entry) {
        return 0;
    }

    strcpy(dirent.name, entry->name);
    dirent.ino = 0;
    return &dirent;
}

static struct fs_node* ramdiskfs_finddir(fs_node_t* node, char* name) {
    uintptr_t base = (uintptr_t) node->ptr->context;
    ramdiskfs_header_t* header = (ramdiskfs_header_t*) base;
    if (header->signature != RDFS_SIGNATURE) {
        return 0;
    }

    uint32_t current_entry = node->ptr == node ? header->root_entry : node->context;
    ramdiskfs_entry_t* entry = (ramdiskfs_entry_t*) (base + current_entry);
    if (!current_entry) {
        return 0;
    }

    current_entry = entry->target_entry;
    while (current_entry) {
        entry = (ramdiskfs_entry_t*) (base + current_entry);
        if (strcmp(entry->name, name) == 0) {
            strcpy(rnode.name, entry->name);
            rnode.mask = 0;
            rnode.uid = 0;
            rnode.gid = 0;
            switch (entry->type) {
                case RDFS_TYPE_FILE:
                    rnode.flags = FS_FILE;
                    rnode.read = ramdiskfs_read;
                    rnode.write = 0;
                    rnode.open = 0;
                    rnode.close = 0;
                    rnode.readdir = 0;
                    rnode.finddir = 0;
                    rnode.length = entry->size;
                    break;

                case RDFS_TYPE_DIRECTORY:
                    rnode.flags = FS_DIRECTORY;
                    rnode.read = 0;
                    rnode.write = 0;
                    rnode.open = 0;
                    rnode.close = 0;
                    rnode.readdir = ramdiskfs_readdir;
                    rnode.finddir = ramdiskfs_finddir;
                    rnode.length = 0;
                    break;

                case RDFS_TYPE_SYMLINK:
                    rnode.flags = FS_SYMLINK;
                    rnode.read = 0;
                    rnode.write = 0;
                    rnode.open = 0;
                    rnode.close = 0;
                    rnode.readdir = 0;
                    rnode.finddir = 0;
                    rnode.length = 0;
                    break;

                default:
                    return 0;
            }
            rnode.inode = 0;
            rnode.context = current_entry;
            rnode.ptr = node->ptr;
            return &rnode;
        }

        current_entry = entry->next_entry;
    }

    return 0;
}

fs_node_t* ramdiskfs_open(void* base) {
    if (!base) {
        return 0;
    }

    ramdiskfs_header_t* header = (ramdiskfs_header_t*) base;
    if (header->signature != RDFS_SIGNATURE) {
        return 0;
    }

    if (header->version != 1) {
        return 0;
    }

    if (header->size < sizeof(ramdiskfs_header_t)) {
        return 0;
    }

    fs_node_t* root = malloc(sizeof(fs_node_t));
    strcpy(root->name, "fs:ramdisk");
    root->mask = 0;
    root->uid = 0;
    root->gid = 0;
    root->flags = FS_DIRECTORY;
    root->inode = 0;
    root->length = 0;
    root->context = (uintptr_t) base;
    root->read = 0;
    root->write = 0;
    root->open = 0;
    root->close = 0;
    root->readdir = ramdiskfs_readdir;
    root->finddir = ramdiskfs_finddir;
    root->ptr = root;

    return root;
}