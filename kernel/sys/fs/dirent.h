#pragma once

#include <stdint.h>

struct dirent {
    uint32_t ino;
    char name[256];
};