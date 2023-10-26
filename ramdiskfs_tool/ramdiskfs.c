#include "ramdiskfs.h"

#include <string.h>

static inline ramdiskfs_entry_t* rdfs_get_entry(ramdiskfs_t* fs, uint32_t offset) {
    return (ramdiskfs_entry_t*) (fs->data + offset);
}

ramdiskfs_t* rdfs_read_filesystem(ramdiskfs_t* fs, uint8_t* data) {
    if (!fs) {
        return 0;
    }

    fs->data = data;
    memcpy(&fs->header, data, sizeof(ramdiskfs_header_t));
    if (fs->header.signature != RDFS_SIGNATURE) {
        return 0;
    }

    if (fs->header.version != 1) {
        return 0;
    }

    if (fs->header.size < sizeof(ramdiskfs_header_t)) {
        return 0;
    }

    return fs;
}

ramdiskfs_entry_t* rdfs_find_entry_in(ramdiskfs_t* fs, ramdiskfs_entry_t* entry, const char* name) {
    entry = rdfs_follow_links(fs, entry);

    if (strcmp(name, ".") == 0) {
        return entry;
    }

    if (entry->type != RDFS_TYPE_DIRECTORY || !entry->target_entry) {
        return 0;
    }

    uint32_t current_entry = entry->target_entry;
    while (current_entry) {
        entry = rdfs_get_entry(fs, current_entry);
        if (strcmp(name, entry->name) == 0) {
            return entry;
        }

        current_entry = entry->next_entry;
    }

    return 0;
}

ramdiskfs_entry_t* rdfs_find_entry(ramdiskfs_t* fs, const char* name) {
    if (!fs || !name) {
        return 0;
    }

    return rdfs_find_entry_in(fs, rdfs_get_entry(fs, fs->header.root_entry), name);
}

ramdiskfs_entry_t* rdfs_follow_link(ramdiskfs_t* fs, ramdiskfs_entry_t* entry) {
    if (entry->type == RDFS_TYPE_SYMLINK) {
        return rdfs_get_entry(fs, entry->target_entry);
    } else {
        return entry;
    }
}

ramdiskfs_entry_t* rdfs_follow_links(ramdiskfs_t* fs, ramdiskfs_entry_t* entry) {
    while (entry->type == RDFS_TYPE_SYMLINK) {
        entry = rdfs_get_entry(fs, entry->target_entry);
    }

    return entry;
}

void* rdfs_file_content(ramdiskfs_t* fs, ramdiskfs_entry_t* entry, uint8_t follow_links) {
    if (follow_links) {
        entry = rdfs_follow_links(fs, entry);
    }

    return (void*) (fs->data + entry->offset + sizeof(ramdiskfs_entry_t));
}