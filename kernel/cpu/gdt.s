.section .text
.code64
.align 8

gdtr: .short 0
      .quad 0

.global gdt_load
gdt_load:
    mov %di, [gdtr]
    mov %rsi, [gdtr + 2]
    lgdt [gdtr]
    push $0x08
    mov $gdt_flush, %rax
    push %rax
    retfq
gdt_flush:
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    mov $0x2B, %ax
    ltr %ax
    ret
