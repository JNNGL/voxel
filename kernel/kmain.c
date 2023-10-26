#include <lib/terminal.h>
#include <lib/multiboot.h>
#include <video/vga_terminal.h>
#include <cpu/gdt.h>
#include <cpu/mmu.h>
#include <cpu/idt.h>
#include <cpu/pic.h>
#include <lib/kprintf.h>
#include <lib/string.h>
#include <lib/alloc.h>
#include <sys/fs/ramdiskfs.h>

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

    if (multiboot->flags & MULTIBOOT_FLAG_MODS) {
        multiboot_module_t* mods = (multiboot_module_t*)(uintptr_t) multiboot->mods_addr;
        for (uint32_t i = 0; i < multiboot->mods_count; ++i) {
            uintptr_t end_address = (uintptr_t) mods[i].mod_end;
            if (end_address > highest_kernel_address) {
                highest_kernel_address = end_address;
            }
        }
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

void ramdisk_dump(fs_node_t* root_node, int level) {
    int i = 0;
    struct dirent* dirent = 0;
    while ((dirent = readdir_fs(root_node, i)) != 0) {
        kprintf("\n");
        for (int i = 0; i < level; i++) {
            kprintf("- ");
        }
        kprintf("%s ", dirent->name);
        fs_node_t* node_ptr = finddir_fs(root_node, dirent->name);
        if (node_ptr) {
            fs_node_t node = *node_ptr;
            switch (node.flags) {
                case FS_FILE:
                    kprintf("(file) ");
                    char buf[1024];
                    buf[read_fs(&node, 0, 1023, buf)] = 0;
                    kprintf("%s ", buf);
                    break;

                case FS_DIRECTORY:
                    kprintf("(directory) ");
                    ramdisk_dump(&node, level + 1);
                    break;

                case FS_SYMLINK:
                    kprintf("(symlink) ");
                    break;
            }
        }
        ++i;
    }
}

void kmain(struct multiboot* mboot, uint32_t magic, uintptr_t esp) {
    if (magic != MULTIBOOT_EAX_MAGIC) {
        puts("Invalid magic.");
        return;
    }

    if (!(mboot->flags & MULTIBOOT_FLAG_MODS)) {
        puts("No modules found.");
        return;
    }

    multiboot = mboot;

    terminal = vga_terminal;
    terminal_init();

    multiboot_initialize();

    mmu_init(multiboot_memory_marker, highest_valid_address, highest_kernel_address);
    heap_init(0x10);

    pat_init();

    gdt_encode_entry(&gdt[0], 0, 0, 0, 0);
    gdt_encode_entry(&gdt[1], 0, UINT32_MAX, 0x9A, 0x02);
    gdt_encode_entry(&gdt[2], 0, UINT32_MAX, 0x92, 0x02);
    gdt_encode_entry(&gdt[3], 0, UINT32_MAX, 0xFA, 0x02);
    gdt_encode_entry(&gdt[4], 0, UINT32_MAX, 0xF2, 0x02);
    gdt_load(sizeof(gdt) - 1, gdt);

    idt_init();
    pic_remap();

    multiboot_module_t* mods = (multiboot_module_t*)(uintptr_t) multiboot->mods_addr;
    fs_node_t* ramdisk_root = ramdiskfs_open(mmu_from_physical(mods->mod_start));

    puts("RAMDISK");
    puts("=======");

    ramdisk_dump(ramdisk_root, 0);

    puts("\n\n=======");

    asm("sti");
    puts("hello, world!");
}