global check_cpuid_asm

check_cpuid_asm:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    cmp eax, ecx
    je no_cpuid
    mov eax, 1
    ret
no_cpuid:
    mov eax, 0
    ret
