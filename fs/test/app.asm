bits 32
org  0x1000000

mov eax, 0x83
mov ebx, message
mov ecx, message_length

int 0x1C

loop:
    jmp loop

message: db 'G', 0ch, 'o', 0ch, 'o', 0ch, 'd', 0ch, ' ', 0ch, 'd', 0fh, 'a', 0fh, 'y', 0fh, '!', 0fh
message_length equ $-message