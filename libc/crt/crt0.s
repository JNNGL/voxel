.global _start
_start:
    movq $main, %rcx
    and $0xFFFFFFFFFFFFFFF0, %rsp
    call _main
