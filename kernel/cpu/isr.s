.code64
.section .text
.align 8

.macro IRQ index byte
    .global _irq\index
    .type _irq\index, @function
    _irq\index:
        pushq $0x00
        pushq $\byte
        jmp isr_common
.endm

.macro ISR_NOERR index
    .global _isr\index
    .type _isr\index, @function
    _isr\index:
        pushq $0x00
        pushq $\index
        jmp isr_common
.endm

.macro ISR_ERR index
    .global _isr\index
    .type _isr\index, @function
    _isr\index:
        pushq $\index
        jmp isr_common
.endm

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_ERR   21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_ERR   29
ISR_ERR   30
ISR_NOERR 31
IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47
ISR_NOERR 128

.global _isr2
.type _isr2, @function
_isr2:
    cli
1:  hlt
    jmp 1b

.global _isr123
.type _isr123, @function
_isr123:
    iretq

.global _isr124
.type _isr124, @function
_isr124:
    iretq

.global _isr125
.type _isr125, @function
_isr125:
    cli:
1:  hlt
    jmp 1b

.global _isr126
.type _isr126, @function
_isr126:
    iretq

.extern isr_handler
.type isr_handler, @function

.global isr_common
isr_common:
    cmpq $8, 24(%rsp)
    je 1f
    swapgs
1:  push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %rbp
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15
    cld
    mov %rsp, %rdi
    call isr_handler
    mov %rax, %rsp
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax
    cmpq $8, 24(%rsp)
    je 2f
    swapgs
2:  add $16, %rsp
    iretq
