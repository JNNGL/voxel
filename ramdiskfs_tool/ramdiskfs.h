#pragma once

#include <stdint.h>

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

typedef struct ramdiskfs {
    ramdiskfs_header_t header;
    uint8_t* data;
} ramdiskfs_t;

ramdiskfs_t* rdfs_read_filesystem(ramdiskfs_t* fs, uint8_t* data);
ramdiskfs_entry_t* rdfs_find_entry_in(ramdiskfs_t* fs, ramdiskfs_entry_t* entry, const char* name);
ramdiskfs_entry_t* rdfs_find_entry(ramdiskfs_t* fs, const char* name);
ramdiskfs_entry_t* rdfs_follow_link(ramdiskfs_t* fs, ramdiskfs_entry_t* entry);
ramdiskfs_entry_t* rdfs_follow_links(ramdiskfs_t* fs, ramdiskfs_entry_t* entry);
void* rdfs_file_content(ramdiskfs_t* fs, ramdiskfs_entry_t* entry, uint8_t follow_links);