global apply_gdt

; gdtptr:
;     dw 0
;     dd 0

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
    dw gdt_end - gdt_start - 1 ; size (16 bit), always one less of its true size
    dd gdt_start

apply_gdt:
    ; mov   eax, [esp + 4]
    ; mov   [gdtptr + 2], eax
    ; mov   ax, [esp + 8]
    ; mov   [gdtptr], ax
    cli
    lgdt  [gdt_descriptor]
    jmp   0x08:reload_segs

reload_segs:
    mov   ax, 0x10
    mov   ds, ax
    mov   es, ax
    mov   fs, ax
    mov   gs, ax
    mov   ss, ax
    ret