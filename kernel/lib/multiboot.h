#pragma once

#include <stdint.h>

#define MULTIBOOT_MAGIC        0x1BADB002
#define MULTIBOOT_EAX_MAGIC    0x2BADB002
#define MULTIBOOT_FLAG_MEM     0x001
#define MULTIBOOT_FLAG_DEVICE  0x002
#define MULTIBOOT_FLAG_CMDLINE 0x004
#define MULTIBOOT_FLAG_MODS    0x008
#define MULTIBOOT_FLAG_AOUT    0x010
#define MULTIBOOT_FLAG_ELF     0x020
#define MULTIBOOT_FLAG_MMAP    0x040
#define MULTIBOOT_FLAG_DRIVE   0x080
#define MULTIBOOT_FLAG_CONFIG  0x100
#define MULTIBOOT_FLAG_LOADER  0x200
#define MULTIBOOT_FLAG_APM     0x400
#define MULTIBOOT_FLAG_VBE     0x800
#define MULTIBOOT_FLAG_FB      0x1000

#define MULTIBOOT_MEMORY_AVAILABLE 1
#define MULTIBOOT_MEMORY_RESERVED  2

struct multiboot {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t num;
    uint32_t size;
    uint32_t addr;
    uint32_t shndx;
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
};

typedef struct vbe_info_s {
    uint16_t attributes;
    uint8_t win_a;
    uint8_t win_b;
    uint16_t granularity;
    uint16_t winsize;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t real_fct_ptr;
    uint16_t pitch;
    uint16_t x_res;
    uint16_t y_res;
    uint8_t w_char;
    uint8_t y_char;
    uint8_t planes;
    uint8_t bpp;
    uint8_t banks;
    uint8_t memory_model;
    uint8_t bank_size;
    uint8_t image_pages;
    uint8_t reserved0;
    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
    uint8_t rsv_mask;
    uint8_t rsv_position;
    uint8_t directcolor_attributes;
    uint32_t physbase;
    uint32_t reserved1;
    uint16_t reserved2;
} vbe_info_t;

typedef struct multiboot_memory_map_entry_s {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed)) multiboot_memory_map_t;