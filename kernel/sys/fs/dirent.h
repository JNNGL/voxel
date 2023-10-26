#pragma once

#include <stdint.h>

struct dirent {
    char name[256];
    uint32_t ino;
};