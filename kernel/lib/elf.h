#pragma once

#include <sys/fs/vfs.h>
#include <stdint.h>
#include <stddef.h>

typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t Elf64_Sword;
typedef uint64_t Elf64_Xword;
typedef int64_t Elf64_Sxword;

#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

#define ELFCLASS32 1
#define ELFCLASS64 2

#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

#define ET_NONE 0
#define ET_REL  1
#define ET_EXEC 2
#define ET_DYN  3
#define ET_CORE 4

#define EI_MAG0       0
#define EI_MAG1       1
#define EI_MAG2       2
#define EI_MAG3       3
#define EI_CLASS      4
#define EI_DATA       5
#define EI_VERSION    6
#define EI_OSABI      7
#define EI_ABIVERSION 8
#define EI_PAD        9
#define EI_NIDENT     16

#define EM_X86_64     62

typedef struct Elf64_Header {
    uint8_t e_ident[EI_NIDENT];
    Elf64_Half e_type;
    Elf64_Half e_machine;
    Elf64_Word e_version;
    Elf64_Addr e_entry;
    Elf64_Off e_phoff;
    Elf64_Off e_shoff;
    Elf64_Word e_flags;
    Elf64_Half e_ehsize;
    Elf64_Half e_phentsize;
    Elf64_Half e_phnum;
    Elf64_Half e_shentsize;
    Elf64_Half e_shnum;
    Elf64_Half e_shstrndx;
} Elf64_Header;

#define SHN_UNDEF   0
#define SHN_LOPROC  0xFF00
#define SHN_HIPROC  0xFF1F
#define SHN_LOOS    0xFF20
#define SHN_HIOS    0xFF3F
#define SHN_ABS     0xFFF1
#define SHN_COMMON  0xFFF2

#define SHT_NULL       0
#define SHT_PROGBITS   1
#define SHT_SYMTAB     2
#define SHT_STRTAB     3
#define SHT_RELA       4
#define SHT_HASH       5
#define SHT_DYNAMIC    6
#define SHT_NOTE       7
#define SHT_NOBITS     8
#define SHT_REL        9
#define SHT_SHLIB      10
#define SHT_DYNSYM     11
#define SHT_LOOS       0x60000000
#define SHT_HIOS       0x6FFFFFFF
#define SHT_LOPROC     0x70000000
#define SHT_HIPROC     0x7FFFFFFF

#define SHF_WRITE          0x00000001
#define SHF_ALLOC          0x00000002
#define SHF_EXECINSTR      0x00000004
#define SHF_MASKOS         0x0F000000
#define SHF_MASKPROC       0xF0000000
#define SHF_X86_64_LARGE   0x10000000
#define SHF_X86_64_UNWIND  0x70000001

typedef struct Elf64_Shdr {
    Elf64_Word sh_name;
    Elf64_Word sh_type;
    Elf64_Xword sh_flags;
    Elf64_Addr sh_addr;
    Elf64_Off sh_offset;
    Elf64_Xword sh_size;
    Elf64_Word sh_link;
    Elf64_Xword sh_addralign;
    Elf64_Xword sh_entsize;
} Elf64_Shdr;

#define STB_LOCAL   0
#define STB_GLOBAL  1
#define STB_WEAK    2
#define STB_LOOS    10
#define STB_HIOS    12
#define STB_LOPROC  13
#define STB_HIPROC  115

#define STT_NOTYPE   0
#define STT_OBJECT   1
#define STT_FUNC     2
#define STT_SECTION  3
#define STT_FILE     4
#define STT_LOOS     10
#define STT_HIOS     12
#define STT_LOPROC   13
#define STT_HIPROC   15

typedef struct Elf64_Sym {
    Elf64_Word st_name;
    uint8_t st_info;
    uint8_t st_other;
    Elf64_Half sh_shndx;
    Elf64_Addr st_value;
    Elf64_Xword st_size;
} Elf64_Sym;

#define ELF64_R_SYM(i) ((i) >> 32)
#define ELF64_R_TYPE(i) ((i) & 0xFFFFFFFFL)
#define ELF64_R_INFO(s,t) (((s) << 32) + ((t) & 0xFFFFFFFFL))

typedef struct Elf64_Rel {
    Elf64_Addr r_offset;
    Elf64_Xword r_info;
} Elf64_Rel;

typedef struct Elf64_Rela {
    Elf64_Addr r_offset;
    Elf64_Xword r_info;
    Elf64_Sxword r_addend;
} Elf64_Rela;

#define R_X86_64_NONE             0
#define R_X86_64_64               1
#define R_X86_64_PC32             2
#define R_X86_64_GOT32            3
#define R_X86_64_PLT32            4
#define R_X86_64_COPY             5
#define R_X86_64_GLOB_DAT         6
#define R_X86_64_JUMP_SLOT        7
#define R_X86_64_RELATIVE         8
#define R_X86_64_GOTPCREL         9
#define R_X86_64_32               10
#define R_X86_64_32S              11
#define R_X86_64_16               12
#define R_X86_64_PC16             13
#define R_X86_64_8                14
#define R_X86_64_PC8              15
#define R_X86_64_DTPMOD64         16
#define R_X86_64_DTPOFF64         17
#define R_X86_64_TPOFF64          18
#define R_X86_64_TLSGD            19
#define R_X86_64_TLSLD            20
#define R_X86_64_DTPOFF32         21
#define R_X86_64_GOTTPOFF         22
#define R_X86_64_TPOFF32          23
#define R_X86_64_PC64             24
#define R_X86_64_GOTOFF64         25
#define R_X86_64_GOTPC32          26
#define R_X86_64_GOT64            27
#define R_X86_64_GOTPCREL64       28
#define R_X86_64_GOTPC64          29
#define R_X86_64_GOTPLT64         30
#define R_X86_64_PLTOFF64         31
#define R_X86_64_SIZE32           32
#define R_X86_64_SIZE64           33
#define R_X86_64_GOTPC32_TLSDESC  34
#define R_X86_64_TLSDESC_CALL     35
#define R_X86_64_TLSDESC          36
#define R_X86_64_IRELATIVE        37

#define R_AARCH64_COPY          1024
#define R_AARCH64_GLOB_DAT      1025

#define PT_NULL     0
#define PT_LOAD     1
#define PT_DYNAMIC  2
#define PT_INTERP   3
#define PT_NOTE     4
#define PT_SHLIB    5
#define PT_PHDR     6
#define PT_TLS      7
#define PT_LOOS     0x60000000
#define PT_HIOS     0x6FFFFFFF
#define PT_LOPROC   0x70000000
#define PT_HIPROC   0x7FFFFFFF
#define PT_GNU_EH_FRAME  0x6474e550
#define PT_SUNW_EH_FRAME 0x6474e550
#define PT_SUNW_UNWIND   0x6464e550

#define PF_X        0x01
#define PF_W        0x02
#define PF_R        0x04
#define PF_MASKOS   0x00FF0000
#define PF_MAKSPROC 0xFF000000

typedef struct Elf64_Phdr {
    Elf64_Word p_type;
    Elf64_Word p_flags;
    Elf64_Off p_offset;
    Elf64_Addr p_vaddr;
    Elf64_Addr p_paddr;
    Elf64_Xword p_filesz;
    Elf64_Xword p_memsz;
    Elf64_Xword p_align;
} Elf64_Phdr;

#define DT_NULL         0
#define DT_NEEDED       1
#define DT_PLTRELSZ     2
#define DT_PLTGOT       3
#define DT_HASH         4
#define DT_STRTAB       5
#define DT_SYMTAB       6
#define DT_RELA         7
#define DT_RELASZ       8
#define DT_RELAENT      9
#define DT_STRSZ        10
#define DT_SYMENT       11
#define DT_INIT         12
#define DT_FINI         13
#define DT_SONAME       14
#define DT_RPATH        15
#define DT_SYMBOLIC     16
#define DT_REL          17
#define DT_RELSZ        18
#define DT_RELENT       19
#define DT_PLTREL       20
#define DT_DEBUG        21
#define DT_TEXTREL      22
#define DT_JMPREL       23
#define DT_BIND_NOW     24
#define DT_INIT_ARRAY   25
#define DT_FINI_ARRAY   26
#define DT_INIT_ARRAYSZ 27
#define DT_FINI_ARRAYSZ 28
#define DT_LOOS   0x60000000
#define DT_HIOS   0x6FFFFFFF
#define DT_LOPROC 0x70000000
#define DT_HIPROC 0x7FFFFFFF

typedef struct Elf64_Dyn {
    Elf64_Sxword d_tag;
    union {
        Elf64_Xword d_val;
        Elf64_Addr d_ptr;
    } d_un;
} Elf64_Dyn;

int elf_exec(const char* path, fs_node_t* file, int argc, const char* argv[], const char* envp[]);