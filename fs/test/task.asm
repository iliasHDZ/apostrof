bits 32
org  0x4000000

loop:
    push 0x69696969

    mov eax, 0x11
    mov ebx, 1
    mov ecx, message
    mov edx, 1

    ;int 0x1C

    add esp, 4

    jmp loop

message: db "TASK 1", 0
message_length equ $-message