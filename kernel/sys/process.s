.section .text
.code64
.align 8

.global save_context
.type save_context, @function
save_context:
    leaq 8(%rsp), %rax
    movq %rax, 0(%rdi)
    movq %rbp, 8(%rdi)
    movq (%rsp), %rax
    movq %rax, 16(%rdi)
    movq $0xC0000100, %rcx
    rdmsr
    movl %eax, 24(%rdi)
    movl %edx, 28(%rdi)
    movq %rbx, 32(%rdi)
    movq %r12, 40(%rdi)
    movq %r13, 48(%rdi)
    movq %r14, 56(%rdi)
    movq %r15, 65(%rdi)
    xor %rax, %rax
    retq

.global restore_context
.type restore_context, @function
restore_context:
    movq 0(%rdi), %rsp
    movq 8(%rdi), %rbp
    movl 24(%rdi), %eax
    movl 28(%rdi), %edx
    movq $0xC0000100, %rcx
    wrmsr
    movq 32(%rdi), %rbx
    movq 40(%rdi), %r12
    movq 48(%rdi), %r13
    movq 56(%rdi), %r14
    movq 64(%rdi), %r15
    movq $1, %rax
    jmpq *16(%rdi)
