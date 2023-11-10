#include "elf.h"

#include <cpu/mmu.h>
#include <cpu/gdt.h>
#include <cpu/userspace.h>
#include <sys/process.h>
#include <sys/errno.h>
#include <lib/alloc.h>
#include <lib/string.h>
#include <lib/terminal.h>

int elf_exec(const char* path, fs_node_t* file, int argc, const char* argv[], const char* envp[]) {
    Elf64_Header header;

    read_fs(file, 0, sizeof(Elf64_Header), (uint8_t*) &header);

    if (header.e_ident[0] != ELFMAG0 ||
        header.e_ident[1] != ELFMAG1 ||
        header.e_ident[2] != ELFMAG2 ||
        header.e_ident[3] != ELFMAG3) {
        close_fs(file);
        free(file);
        puts("elf: invalid magic");
        return 0;
    }

    if (header.e_ident[EI_CLASS] != ELFCLASS64) {
        puts("elf: invalid class");
        return 0;
    }

    if (header.e_type != ET_EXEC) {
        puts("elf: invalid type");
        return 0;
    }

    uintptr_t heap_base = 0;

    mmu_set_directory(0);
    pagemap_entry_t* current_directory = current_process->thread.page_directory;
    current_process->thread.page_directory = mmu_clone(0);
    mmu_set_directory(current_process->thread.page_directory);
    mmu_free(current_directory);

    for (int i = 0; i < header.e_phnum; ++i) {
        Elf64_Phdr phdr;
        read_fs(file, header.e_phoff + header.e_phentsize * i, sizeof(Elf64_Phdr), (uint8_t*) &phdr);
        if (phdr.p_type == PT_LOAD) {
            for (uintptr_t j = phdr.p_vaddr; j < phdr.p_vaddr + phdr.p_memsz; j += 0x1000) {
                pagemap_entry_t* page = mmu_lookup_frame(j, MMU_GET_MAKE);
                mmu_allocate_frame_ex(page, MMU_FLAG_WRITABLE);
            }

            read_fs(file, phdr.p_offset, phdr.p_filesz, (void*) phdr.p_vaddr);

            for (size_t j = phdr.p_filesz; j < phdr.p_memsz; ++j) {
                *(char*)(phdr.p_vaddr + j) = 0;
            }

            if (phdr.p_vaddr + phdr.p_memsz > heap_base) {
                heap_base = phdr.p_vaddr + phdr.p_memsz;
            }
        }
    }

    current_process->image.heap = (heap_base + 0xFFF) & (~0xFFF);
    current_process->image.entry = header.e_entry;

    close_fs(file);

    uintptr_t userstack = 0x800000000000;
    for (uintptr_t i = userstack - 512 * 0x400; i < userstack; i += 0x1000) {
        pagemap_entry_t* page = mmu_lookup_frame(i, MMU_GET_MAKE);
        mmu_allocate_frame_ex(page, MMU_FLAG_WRITABLE);
    }

    current_process->image.userstack = userstack - 16 * 0x400;

#define USERSTACK_PUSH(type, val) {        \
    userstack -= sizeof(type);             \
    while (userstack & (sizeof(type) - 1)) \
        --userstack;                       \
    *((type*) userstack) = val;            \
}

#define USERSTACK_PUSHSTR(s) {             \
    for (int l = strlen(s); l >= 0; l--) { \
        USERSTACK_PUSH(char, s[l]);        \
    }                                      \
}

    char* argv_ptrs[argc];
    for (int i = 0; i < argc; ++i) {
        USERSTACK_PUSHSTR(argv[i]);
        argv_ptrs[i] = (char*) userstack;
    }

    int envc = 0;
    char** envpp = (char**) envp;
    while (*envpp) {
        ++envc;
        ++envpp;
    }

    char* envp_ptrs[envc];
    for (int i = 0; i < envc; ++i) {
        USERSTACK_PUSHSTR(envp[i]);
        envp_ptrs[i] = (char*) userstack;
    }

    USERSTACK_PUSH(uintptr_t, 0);
    USERSTACK_PUSH(uintptr_t, current_process->uid);
    USERSTACK_PUSH(uintptr_t, 11);
    USERSTACK_PUSH(uintptr_t, current_process->uid);
    USERSTACK_PUSH(uintptr_t, 12);
    USERSTACK_PUSH(uintptr_t, 0);

    USERSTACK_PUSH(uintptr_t, 0);
    for (int i = envc; i > 0; i--) {
        USERSTACK_PUSH(char*, envp_ptrs[i - 1]);
    }
    char** user_envp = (char**) userstack;

    USERSTACK_PUSH(uintptr_t, 0);
    for (int i = argc; i > 0; i--) {
        USERSTACK_PUSH(char*, argv_ptrs[i - 1]);
    }
    char** user_argv = (char**) userstack;

    USERSTACK_PUSH(uintptr_t, argc);

    set_kernel_stack(current_process->image.stack);
    enter_userspace(header.e_entry, argc, user_argv, user_envp, userstack);

    return -EINVAL;
}