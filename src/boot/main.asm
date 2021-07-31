global start
extern kernel_main

section .kernel_header
dw "KN"
dd start

section .text
bits 32

start:
    mov esp, stack_top

    call kernel_main

    jmp $

section .bss
stack_bottom:
    resb 4096 * 16
stack_top: