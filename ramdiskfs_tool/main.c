#include "ramdiskfs.h"

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

uint8_t BUFFER[1024];

static uint32_t create_directory(FILE* file, const char* path) {
    uint32_t written_bytes = 0;

    DIR* dir = opendir(path);
    if (!dir) {
        perror("can't open directory");
        return 0;
    }

    struct dirent* dirent = readdir(dir);
    while (1) {
        if (strcmp(dirent->d_name, ".") && strcmp(dirent->d_name, "..")) {
            ramdiskfs_entry_t child;
            strcpy(child.name, dirent->d_name);
            child.offset = ftell(file);
            child.next_entry = 0;
            child.target_entry = 0;
            child.size = 0;

            fseek(file, sizeof(ramdiskfs_entry_t), SEEK_CUR);
            char* target = calloc(strlen(path) + strlen(dirent->d_name) + 2, 1);
            sprintf(target, "%s/%s", path, dirent->d_name);
            if (dirent->d_type == DT_REG) {
                child.type = RDFS_TYPE_FILE;
                FILE* child_file = fopen(target, "rb");
                while (1) {
                    uint32_t written = fread(BUFFER, 1, sizeof(BUFFER), child_file);
                    if (written <= 0) {
                        break;
                    }

                    child.size += written;
                    fwrite(BUFFER, 1, written, file);
                }

                fclose(child_file);
            } else if (dirent->d_type == DT_DIR) {
                child.type = RDFS_TYPE_DIRECTORY;
                child.size = create_directory(file, target);
                if (child.size != 0) {
                    child.target_entry = child.offset + sizeof(ramdiskfs_entry_t);
                }
            } else if (dirent->d_type == DT_LNK) {
                // TODO: Implement links
                printf("note: skipping link %s/%s\n", path, dirent->d_name);
                goto skip_entry;
            } else {
                goto skip_entry;
            }

            free(target);

            if ((dirent = readdir(dir))) {
                child.next_entry = child.offset + sizeof(ramdiskfs_entry_t) + child.size;
            }

            written_bytes += sizeof(ramdiskfs_entry_t);
            written_bytes += child.size;

            if (child.type == RDFS_TYPE_DIRECTORY) {
                child.size = 0;
            }

            uint32_t current_pos = ftell(file);
            fseek(file, child.offset, SEEK_SET);
            fwrite(&child, 1, sizeof(ramdiskfs_entry_t), file);
            fseek(file, current_pos, SEEK_SET);

            if (!dirent) {
                break;
            }

            continue;

            skip_entry:
            free(target);
            fseek(file, -sizeof(ramdiskfs_entry_t), SEEK_CUR);
        } else {
            if (!(dirent = readdir(dir))) {
                break;
            }
        }
    }

    closedir(dir);
    return written_bytes;
}

int strstrw(const char* str1, const char* str2) {
    return !strncmp(str1, str2, strlen(str2));
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("usage: misha.mkvfs [--label=] source target\n");
        return -1;
    }

    const char* label = "";
    if (strstrw(argv[1], "--label=")) {
        label = argv[1] + 8;
        if (strlen(label) > 16) {
            fprintf(stderr, "label is too long.\n");
            return -1;
        }

        argv++;
    }

    FILE* image = fopen(argv[2], "wb");
    if (!image) {
        perror("can't open target file");
        return -1;
    }

    ramdiskfs_entry_t root_entry;
    root_entry.name[0] = '\0';
    root_entry.offset = sizeof(ramdiskfs_header_t);
    root_entry.type = RDFS_TYPE_DIRECTORY;
    root_entry.next_entry = 0;
    root_entry.target_entry = root_entry.offset + sizeof(ramdiskfs_entry_t);
    root_entry.size = 0;

    ramdiskfs_header_t header;
    header.signature = RDFS_SIGNATURE;
    header.version = 1;
    strcpy(header.label, label);
    header.root_entry = root_entry.offset;

    fseek(image, sizeof(ramdiskfs_header_t), SEEK_CUR);
    fwrite(&root_entry, 1, sizeof(ramdiskfs_entry_t), image);
    header.size = root_entry.target_entry + create_directory(image, argv[1]);
    fseek(image, 0, SEEK_SET);
    fwrite(&header, 1, sizeof(ramdiskfs_header_t), image);

    fclose(image);

    return 0;
}