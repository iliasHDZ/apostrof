global start
global last_kernel_cookie

extern kernel_main

section .kernel_header
dw "KN"
dd start

section .kernel_footer
last_kernel_cookie: dd 0xAABBCCDD

section .text
bits 32

start:
    mov esp, init_stack_top

    push stack_top
    call kernel_main
    add esp, 4

    sti

loop:
    hlt
    jmp loop

section .bss
stack_bottom:
    resb 4096 * 16
stack_top:

section .bss
init_stack_bottom:
    resb 4096 * 16
init_stack_top: