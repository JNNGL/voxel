#include <lib/terminal.h>
#include <video/vga_terminal.h>
#include <cpu/gdt.h>

uint64_t initial_pages[3][512] __attribute__((aligned(0x1000))) = {0};
uint64_t high_base_pml[512] __attribute__((aligned(0x1000))) = {0};
uint64_t heap_base_pml[512] __attribute__((aligned(0x1000))) = {0};
uint64_t heap_base_pd[512] __attribute__((aligned(0x1000))) = {0};
uint64_t heap_base_pt[512 * 3] __attribute__((aligned(0x1000))) = {0};
uint64_t low_base_pmls[34][512] __attribute__((aligned(0x1000))) = {0};
uint64_t twom_high_pds[64][512] __attribute__((aligned(0x1000))) = {0};

gdt_entry_t gdt[5] = {0};

void kmain(void* multiboot) {
    terminal = vga_terminal;
    terminal_init();

    gdt_encode_entry(&gdt[0], 0, 0, 0, 0);
    gdt_encode_entry(&gdt[1], 0, UINT32_MAX, 0x9A, 0x02);
    gdt_encode_entry(&gdt[2], 0, UINT32_MAX, 0x92, 0x02);
    gdt_encode_entry(&gdt[3], 0, UINT32_MAX, 0xFA, 0x02);
    gdt_encode_entry(&gdt[4], 0, UINT32_MAX, 0xF2, 0x02);
    gdt_load(sizeof(gdt) - 1, gdt);

    puts("Hello, world!");

    while (1);
}