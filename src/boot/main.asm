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
    cli
    lgdt [gdt_descriptor]
    jmp CODE_SEG:gdt_changed

gdt_changed:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov esp, init_stack_top
    mov ebp, esp

    push stack_top
    call kernel_main
    add esp, 4

    sti

loop:
    hlt
    jmp loop

gdt_start:
    dd 0x0
    dd 0x0

gdt_code: 
    dw 0xffff
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0

gdt_data:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

section .bss
stack_bottom:
    resb 4096 * 16
stack_top:

section .bss
init_stack_bottom:
    resb 4096 * 16
init_stack_top: