global start
global last_kernel_cookie

extern kernel_main

section .kernel_header
dw "KN"
dd start

section .text
bits 32

start:
    mov esp, stack_top

    call kernel_main

    sti

loop:
    hlt
    jmp loop

section .last_data
last_kernel_cookie: dd 0xAABBCCDD

section .bss
stack_bottom:
    resb 4096 * 16
stack_top: