#pragma once

#include <stdint.h>

#define PSF1_FONT_MAGIC 0x0436

typedef struct psf_header {
    uint16_t magic;
    uint8_t font_mode;
    uint8_t character_size;
} psf_header_t;

typedef struct psf_font {
    psf_header_t header;
    uint8_t buffer[256 * 256];
} psf_font_t;

psf_font_t* psf_load_font(uint8_t* p);