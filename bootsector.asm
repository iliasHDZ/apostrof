section .boot
org 0x7c00

bits 16

mov bp, 0x9000
mov sp, bp

; disk read
mov ah, 0x02
mov al, 0x80
mov cl, 0x02
mov ch, 0x00
mov dh, 0x00

mov bx, 0x9000

int 0x13
jc disk_error

mov ax, [0x9000]
cmp ax, 0x4E4B

jne kernel_error

cli
lgdt [gdt_descriptor]
mov eax, cr0
or eax,  0x1
mov cr0, eax
jmp CODE_SEG:prot_mode

bits 32
prot_mode:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    jmp DWORD [0x9002]

bits 16
disk_error:
    mov bx, DISK_ERROR
    call print
    jmp loop

kernel_error:
    mov bx, INVALID_KERNEL
    call print

loop:
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

print: ; bx => BYTE PTR string 
    pusha
    mov al, [bx]
    cmp al, 0
    je print_end

    mov ah, 0x0e
    int 0x10

    add bx, 1
    jmp print
print_end:
    popa
    ret

DISK_ERROR: db "Disk Error", 0
INVALID_KERNEL: db "Invalid Kernel", 0

times 510-($-$$) db 0
dw 0xaa55