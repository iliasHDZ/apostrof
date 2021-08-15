bits 32
org  0x4000000

mov eax, 0x80
int 0x1C

cmp eax, 0
je stop

mov [fd], eax

mov eax, 0x11
mov ebx, [fd]
mov ecx, message
mov edx, message_length

int 0x1C

stop:
    jmp stop

fd: dd 0

message: db "TASK 2", 0
message_length equ $-message