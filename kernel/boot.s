.set MULTIBOOT_PAGE_ALIGN,   1<<0
.set MULTIBOOT_MEM_INFO,     1<<1
.set MULTIBOOT_HEADER_MAGIC, 0x1BADB002
.set MULTIBOOT_HEADER_FLAGS, MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEM_INFO
.set MULTIBOOT_CHECKSUM,     -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

.code32

.global multiboot
.extern code
.extern bss
.extern end

.section .multiboot
multiboot:
    .long MULTIBOOT_HEADER_MAGIC
    .long MULTIBOOT_HEADER_FLAGS
    .long MULTIBOOT_CHECKSUM
    .long multiboot
    .long code
    .long bss
    .long end
    .long start

.section .stack, "aw", @nobits
stack_bottom:
.skip 16384
.global stack_top
stack_top:

.section .bootstrap
.align 4

.global start
start:
    mov $stack_top, %esp
    and $-16, %esp

    pushl $0
    pushl %esp
    pushl $0
    pushl %eax
    pushl $0
    pushl %ebx

    mov $initial_pages, %edi

    mov $0x1007, %eax
    add %edi, %eax
    mov %eax, (%edi)

    add $0x1000, %edi
    mov $0x1003, %eax
    add %edi, %eax
    mov %eax, (%edi)

    add $0x1000, %edi

    mov $0x87, %ebx
    mov $32, %ecx

.set_entry:
    mov %ebx, (%edi)
    add $0x200000, %ebx
    add $8, %edi
    loop .set_entry

    mov $initial_pages, %edi
    mov %edi, %cr3

    mov %cr4, %eax
    or $32, %eax
    mov %eax, %cr4

    mov $0xC0000080, %ecx
    rdmsr
    or $256, %eax
    wrmsr

    mov %cr0, %eax
    test $0x80000000, %eax
    jnz .continue
    or $0x80000000, %eax
    mov %eax, %cr0

    lgdt gdtr
    ljmp $0x08, $long_entry

.extern initial_pages

.align 8
gdtr:
    .word gdt_end - gdt_base
    .quad gdt_base

gdt_base:
    .quad 0      // NULL
    .word 0      // CODE
    .word 0
    .byte 0
    .byte 0x9A
    .byte 0x20
    .byte 0
    .word 0xFFFF // DATA
    .word 0
    .byte 0
    .byte 0x92
    .byte 0
    .byte 0
gdt_end:

.code64
.align 8

.extern kmain
long_entry:
    cli
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
.continue:
    cli
    pop %rdi
    pop %rsi
    pop %rdx
    callq kmain
1:  hlt
    jmp 1b
