 .text
    .globl _fast_align_up

#ifdef __APPLE__
    .globl _fast_align_up
_fast_align_up:
#else
    .globl fast_align_up
fast_align_up:
#endif
    addq $15, %rdi
    andq $-16, %rdi
    movq %rdi, %rax
    ret