#include <lib/terminal.h>
#include <lib/multiboot.h>
#include <video/vga_terminal.h>
#include <video/lfb_terminal.h>
#include <video/psf_font.h>
#include <video/lfb.h>
#include <cpu/gdt.h>
#include <cpu/mmu.h>
#include <cpu/idt.h>
#include <cpu/pic.h>
#include <cpu/kbd.h>
#include <cpu/timer.h>
#include <sys/fs/dev/zero.h>
#include <sys/fs/dev/tty.h>
#include <sys/fs/ramdiskfs.h>
#include <sys/fs/tmpfs.h>
#include <sys/fs/vfs.h>
#include <sys/process.h>
#include <lib/kprintf.h>
#include <lib/string.h>
#include <lib/alloc.h>
#include <lib/elf.h>

#define TIMER_FREQ_HZ 50
#define FONT_PATH "/usr/share/fonts/zap-vga16.psf"

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
    while ((dirent = readdir_fs(from, i++)) != 0) {
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
                kprintf("fs: unable to create file in <...>/%s\n", to->name);
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

void terminal_pty_putchar(pty_t* pty, uint8_t c) {
    putchar(c);
}

void dir_dump(fs_node_t* root_node, int level, const char* path) {
    int i = 0;
    struct dirent* dirent = readdir_fs(root_node, 0);
    while (dirent != 0) {
        if (*dirent->name == '.') {
            goto next;
        }
        if (level > 0) {
            kprintf("\n");
        }
        for (int i = 0; i < level - 1; i++) {
            kprintf("  ");
        }
        if (level) {
            kprintf(" - ");
        }
        uint8_t has_name = 1;
        kprintf("%s ", dirent->name);
        char file_path[512];
        char* path_ptr = strcpy(file_path, path) + strlen(path);
        *path_ptr++ = '/';
        strcpy(path_ptr, dirent->name);
        fs_node_t* node = vfs_get_mountpoint(file_path);
        if (!node) {
            node = finddir_fs(root_node, dirent->name);
        }
        if (node) {
            if (node->flags & FS_DIRECTORY) {
                kprintf("(directory) ");
                dir_dump(node, level + 1, file_path);
            }
            free(node);
        } else {
            kprintf("(invalid) ");
        }
        next:
        free(dirent);
        dirent = readdir_fs(root_node, ++i);
        if (level == 0 && dirent && has_name) {
            kprintf("\n");
        }
    }

    if (level == 0) {
        kprintf("\n");
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
    puts("boot: initialized multiboot");

    mmu_init(multiboot_memory_marker, highest_valid_address, highest_kernel_address);
    heap_init(0x10);

    multiboot = mmu_from_physical((uintptr_t) multiboot);

    pat_init();
    puts("cpu: initialized memory");

    extern void* stack_top;
    gdt_encode_entry(&gdt[0], 0, 0, 0, 0);
    gdt_encode_entry(&gdt[1], 0, UINT32_MAX, 0x9A, 0x02);
    gdt_encode_entry(&gdt[2], 0, UINT32_MAX, 0x92, 0x02);
    gdt_encode_entry(&gdt[3], 0, UINT32_MAX, 0xFA, 0x02);
    gdt_encode_entry(&gdt[4], 0, UINT32_MAX, 0xF2, 0x02);
    tss_encode(&gdt[5], &tss, stack_top);
    gdt_load(sizeof(gdt) - 1, gdt);
    puts("cpu: initialized global descriptors");

    idt_init();
    pic_remap();
    puts("cpu: initialized interrupts");

    process_tree_init();
    puts("proc: initialized process tree");
    multitasking_init();
    puts("proc: initialized multitasking");

    vfs_init();
    puts("fs: initialized mountpoints");

    multiboot_module_t* mods = (multiboot_module_t*)(uintptr_t) multiboot->mods_addr;
    fs_node_t* ramdisk_root = ramdiskfs_open(mmu_from_physical(mods->mod_start));
    puts("fs: loaded ramdisk");

    fs_root = tmpfs_create("/");
    puts("fs: mounted tmpfs on /");

    copy_directory(fs_root, ramdisk_root);
    puts("fs: copied ramdisk contents to /");

    vbe_info_t* vbe_info = ((vbe_info_t*) ((uintptr_t) multiboot->vbe_mode_info));
    linear_framebuffer = mmu_from_physical(vbe_info->physbase);
    lfb_width = vbe_info->x_res;
    lfb_height = vbe_info->y_res;
    puts("video: initialized linear framebuffer");

    fs_node_t* node = kopen(FONT_PATH, 0);
    void* font = malloc(node->length);
    read_fs(node, 0, node->length, font);
    close_fs(node);
    free(node);
    kprintf("video: loaded font from %s\n", FONT_PATH);

    size_t lfb_size = lfb_width * lfb_height * 4;
    size_t lfb_pages = (lfb_size + 0x1000) / 0x1000;
    void* lfb_membuffer = (void*) (mmu_request_frames(lfb_pages) << 12);
    for (size_t i = 0; i < lfb_pages; i++) {
        mmu_lock_frame((uintptr_t) lfb_membuffer + i * 0x1000);
    }
    lfb_membuffer = mmu_from_physical((uintptr_t) lfb_membuffer);
    memset(lfb_membuffer, 0, lfb_width * lfb_height * 4);
    lfb_set_membuffer(lfb_membuffer);
    puts("lfb: initialized membuffer");

    terminal = lfb_terminal;
    lfb_terminal_init();
    lfb_terminal_set_font(psf_load_font(font));
    lfb_copy_from_vga();
    puts("video: initialized lfb terminal");

    mkdir("/tmp", 0777);
    puts("fs: initialized /tmp");

    mkdir("/dev", 0); // TODO: devfs?
    puts("fs: initialized /dev");

    zerofs_mount();
    puts("fs: mounted /dev/null and /dev/zero");

    pty_init();
    puts("fs: initialized /dev/tty");

    pty_t* pty = pty_new(0, 1);
    pty->write_out = terminal_pty_putchar;
    pty->ios.oflag &= ~ONLCR;
    puts("pts: initialized /dev/pts/1");

    kbd_init(pty->master);
    puts("kbd: bound to pty 1");

    timer_init(TIMER_FREQ_HZ);
    kprintf("cpu: initialized timer; frequency: %dHz\n", TIMER_FREQ_HZ);

    kprintf("\nfile hierarchy:\n\n");
    dir_dump(fs_root, 0, "");
    kprintf("\n");

    const char* init_path = "/bin/init";
    fs_node_t* init_node = kopen(init_path, 0);
    if (init_node) {
        kprintf("proc: yielding control to %s\n", init_path);
        const char* argv[] = {init_path};
        const char* envp[] = {0};
        elf_exec(init_path, init_node, sizeof(argv) / sizeof(*argv), argv, envp);
    } else {
        kprintf("fs: %s not found.", init_path);
    }
}