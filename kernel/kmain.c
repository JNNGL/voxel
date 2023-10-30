#include <lib/terminal.h>
#include <lib/multiboot.h>
#include <video/vga_terminal.h>
#include <cpu/gdt.h>
#include <cpu/mmu.h>
#include <cpu/idt.h>
#include <cpu/pic.h>
#include <cpu/timer.h>
#include <sys/fs/ramdiskfs.h>
#include <sys/fs/tmpfs.h>
#include <sys/fs/vfs.h>
#include <sys/process.h>
#include <lib/kprintf.h>
#include <lib/string.h>
#include <lib/alloc.h>
#include <lib/elf.h>

static struct multiboot* multiboot;
static gdt_entry_t gdt[7] = {0};
static tss_entry_t tss;

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

void copy_directory(fs_node_t* to, fs_node_t* from) {
    if (!to) {
        puts("fs: invalid target directory");
        return;
    }

    if (!from) {
        puts("fs: invalid input directory");
        return;
    }

    struct dirent* dirent;
    size_t i = 0;
    while (1) {
        dirent = readdir_fs(from, i++);
        if (!dirent) {
            return;
        }

        kprintf("fs: copying <...>/%s/%s\n", from->name, dirent->name);

        fs_node_t* node = finddir_fs(from, dirent->name);
        if (!node) {
            free(dirent);
            kprintf("fs: unable to find file <...>/%s/%s\n", from->name, dirent->name);
            continue;
        }

        open_fs(node, 0);
        if (node->flags & FS_FILE) {
            if (create_fs(to, node->name, 0777)) {
                kprintf("fs: unable to create file <...>/%s/%s\n", from->name, node->name);
                goto skip;
            }
            fs_node_t* target = finddir_fs(to, node->name);
            if (!target) {
                kprintf("fs: unable to create file in <...>/%s", to->name);
                goto skip;
            }
            open_fs(target, 0);
            uint8_t buf[1024];
            size_t offset = 0;
            size_t remaining = node->length;
            while (remaining) {
                size_t to_copy = remaining > sizeof(buf) ? sizeof(buf) : remaining;
                to_copy = read_fs(node, offset, to_copy, buf);
                if (to_copy > sizeof(buf) || to_copy > remaining) {
                    kprintf("fs: unable to read file <...>/%s/%s\n", from->name, node->name);
                    close_fs(target);
                    free(target);
                    goto skip;
                }
                size_t to_write = to_copy;
                while (to_write) {
                    size_t written = write_fs(target, offset + to_copy - to_write, to_write, buf);
                    if (written > to_write) {
                        kprintf("fs: unable to write file <...>/%s/%s\n", from->name, node->name);
                        close_fs(target);
                        free(target);
                        goto skip;
                    }

                    to_write -= written;
                }
                remaining -= to_copy;
                offset += to_copy;
            }
            close_fs(target);
            free(target);
        } else if (node->flags & FS_DIRECTORY) {
            if (mkdir_fs(to, node->name, 0777)) {
                kprintf("fs: unable to create directory <...>/%s/%s\n", from->name, node->name);
                goto skip;
            }
            fs_node_t* target = finddir_fs(to, node->name);
            if (!target) {
                kprintf("fs: unable to create directory in <...>/%s/\n", from->name);
                goto skip;
            }
            open_fs(target, 0);
            copy_directory(target, node);
            close_fs(target);
            free(target);
        } else {
            kprintf("fs: skipping file <...>/%s/%s\n", from->name, node->name);
        }

        skip:
        close_fs(node);
        free(dirent);
        free(node);
    }
}

void kmain(struct multiboot* mboot, uint32_t magic, uintptr_t esp) {
    if (magic != MULTIBOOT_EAX_MAGIC) {
        puts("multiboot: invalid magic.");
        return;
    }

    if (!(mboot->flags & MULTIBOOT_FLAG_MODS)) {
        puts("multiboot: no modules found.");
        return;
    }

    multiboot = mboot;

    terminal = vga_terminal;
    terminal_init();
    puts("video: initialized vga terminal");

    multiboot_initialize();
    puts("cpu: initialized multiboot");

    mmu_init(multiboot_memory_marker, highest_valid_address, highest_kernel_address);
    heap_init(0x10);
    puts("cpu: initialized mmu/heap");

    multiboot = mmu_from_physical((uintptr_t) multiboot);

    pat_init();

    extern void* stack_top;
    gdt_encode_entry(&gdt[0], 0, 0, 0, 0);
    gdt_encode_entry(&gdt[1], 0, UINT32_MAX, 0x9A, 0x02);
    gdt_encode_entry(&gdt[2], 0, UINT32_MAX, 0x92, 0x02);
    gdt_encode_entry(&gdt[3], 0, UINT32_MAX, 0xFA, 0x02);
    gdt_encode_entry(&gdt[4], 0, UINT32_MAX, 0xF2, 0x02);
    tss_encode(&gdt[5], &tss, stack_top);
    gdt_load(sizeof(gdt) - 1, gdt);
    puts("cpu: initialized gdt");

    idt_init();
    pic_remap();
    puts("cpu: initialized interrupts");

    process_tree_init();
    puts("proc: initialized process tree");
    multitasking_init();
    puts("proc: initialized multitasking");

    multiboot_module_t* mods = (multiboot_module_t*)(uintptr_t) multiboot->mods_addr;
    fs_node_t* ramdisk_root = ramdiskfs_open(mmu_from_physical(mods->mod_start));
    puts("fs: loaded ramdisk");

    fs_root = tmpfs_create("/");
    puts("fs: mounted tmpfs on /");

    copy_directory(fs_root, ramdisk_root);
    puts("fs: copied ramdisk contents to /");

    timer_init(50);

    const char* init_path = "/bin/init";
    fs_node_t* init_node = kopen(init_path, 0);
    if (init_node) {
        kprintf("proc: yielding control to %s\n", init_path);
        const char* argv[] = {init_path};
        const char* envp[] = {0};
        elf_exec(init_path, init_node, 0, argv, envp);
    } else {
        kprintf("fs: %s not found.", init_path);
    }
}