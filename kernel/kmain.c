#include <lib/terminal.h>
#include <lib/multiboot.h>
#include <video/vga_terminal.h>
#include <cpu/gdt.h>
#include <cpu/mmu.h>
#include <cpu/idt.h>
#include <cpu/pic.h>
#include <lib/kprintf.h>
#include <lib/string.h>

static struct multiboot* multiboot;
static gdt_entry_t gdt[5] = {0};

extern char end[];
static uintptr_t highest_valid_address = 0;
static uintptr_t highest_kernel_address = (uintptr_t) &end;

void multiboot_memory_marker() {
    multiboot_memory_map_t* mmap = mmu_from_physical((uintptr_t) multiboot->mmap_addr);
    while ((uintptr_t) mmap < (uintptr_t) mmu_from_physical(multiboot->mmap_addr + multiboot->mmap_length)) {
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            for (uintptr_t base = mmap->addr; base < mmap->addr + (mmap->len & 0xFFFFFFFFFFFFF000); base += 0x1000) {
                mmu_release_frame(base);
            }
        }

        mmap = (multiboot_memory_map_t*) ((uintptr_t) mmap + mmap->size + sizeof(uint32_t));
    }
}

void multiboot_initialize() {
    if (!(multiboot->flags & MULTIBOOT_FLAG_MMAP)) {
        return;
    }

    multiboot_memory_map_t* mmap = (void*)(uintptr_t) multiboot->mmap_addr;

    if ((uintptr_t) mmap + multiboot->mmap_length > highest_kernel_address) {
        highest_kernel_address = (uintptr_t) mmap + multiboot->mmap_length;
    }

    while ((uintptr_t) mmap < multiboot->mmap_addr + multiboot->mmap_length) {
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE && mmap->len && mmap->addr + mmap->len - 1 > highest_valid_address) {
            highest_valid_address = mmap->addr + mmap->len - 1;
        }

        mmap = (multiboot_memory_map_t*) ((uintptr_t) mmap + mmap->size + sizeof(uint32_t));
    }

    highest_kernel_address = (highest_kernel_address + 0xFFF) & 0xFFFFFFFFFFFFF000;
}

void pat_init() {
    asm volatile("mov $0x277, %%ecx\n"
                 "rdmsr\n"
                 "or $0x1000000, %%edx\n"
                 "and $0xF9FFFFFF, %%edx\n"
                 "wrmsr" : : : "ecx", "edx", "eax");
}

void kmain(struct multiboot* mboot, uint32_t magic, uintptr_t esp) {
    if (magic != MULTIBOOT_EAX_MAGIC) {
        puts("Invalid magic.");
        return;
    }

    multiboot = mboot;

    terminal = vga_terminal;
    terminal_init();

    multiboot_initialize();

    mmu_init(multiboot_memory_marker, highest_valid_address, highest_kernel_address);

    pat_init();

    gdt_encode_entry(&gdt[0], 0, 0, 0, 0);
    gdt_encode_entry(&gdt[1], 0, UINT32_MAX, 0x9A, 0x02);
    gdt_encode_entry(&gdt[2], 0, UINT32_MAX, 0x92, 0x02);
    gdt_encode_entry(&gdt[3], 0, UINT32_MAX, 0xFA, 0x02);
    gdt_encode_entry(&gdt[4], 0, UINT32_MAX, 0xF2, 0x02);
    gdt_load(sizeof(gdt) - 1, gdt);

    idt_init();
    pic_remap();

    asm("sti");
    puts("hello, world!");
}