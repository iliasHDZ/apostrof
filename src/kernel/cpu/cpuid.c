#include "cpuid.h"

extern u32 check_cpuid_asm();

u8 cpuid_check() {
    return check_cpuid_asm();
}

u8 cmp_id(const char* id1, const char* id2) {
    for (int i = 0; i < 12; i++)
        if (id1[i] != id2[i]) return 0;

    return 1;
}

const char* cpuid_vendor() {
    u32 a = 0x00;
    u32 b, c, d;

    char id[12];
    
    asm("cpuid"
        : "=a" (a), "=b" (b), "=c" (c), "=d" (d)
        );

    *(u32*)(id + 0) = b;
    *(u32*)(id + 4) = d;
    *(u32*)(id + 8) = c;

    if (cmp_id(id, CPUID_VENDOR_OLDAMD))       return CPUID_VENDOR_OLDAMD;
    if (cmp_id(id, CPUID_VENDOR_AMD))          return CPUID_VENDOR_AMD;
    if (cmp_id(id, CPUID_VENDOR_INTEL))        return CPUID_VENDOR_INTEL;
    if (cmp_id(id, CPUID_VENDOR_CENTAUR))      return CPUID_VENDOR_CENTAUR;
    if (cmp_id(id, CPUID_VENDOR_OLDTRANSMETA)) return CPUID_VENDOR_OLDTRANSMETA;
    if (cmp_id(id, CPUID_VENDOR_TRANSMETA))    return CPUID_VENDOR_TRANSMETA;
    if (cmp_id(id, CPUID_VENDOR_CYRIX))        return CPUID_VENDOR_CYRIX;
    if (cmp_id(id, CPUID_VENDOR_CENTAUR))      return CPUID_VENDOR_CENTAUR;
    if (cmp_id(id, CPUID_VENDOR_NEXGEN))       return CPUID_VENDOR_NEXGEN;
    if (cmp_id(id, CPUID_VENDOR_UMC))          return CPUID_VENDOR_UMC;
    if (cmp_id(id, CPUID_VENDOR_SIS))          return CPUID_VENDOR_SIS;
    if (cmp_id(id, CPUID_VENDOR_NSC))          return CPUID_VENDOR_NSC;
    if (cmp_id(id, CPUID_VENDOR_RISE))         return CPUID_VENDOR_RISE;
    if (cmp_id(id, CPUID_VENDOR_VORTEX))       return CPUID_VENDOR_VORTEX;
    if (cmp_id(id, CPUID_VENDOR_VIA))          return CPUID_VENDOR_VIA;
    if (cmp_id(id, CPUID_VENDOR_VMWARE))       return CPUID_VENDOR_VMWARE;
    if (cmp_id(id, CPUID_VENDOR_XENHVM))       return CPUID_VENDOR_XENHVM;
    if (cmp_id(id, CPUID_VENDOR_MICROSOFT_HV)) return CPUID_VENDOR_MICROSOFT_HV;
    if (cmp_id(id, CPUID_VENDOR_PARALLELS))    return CPUID_VENDOR_PARALLELS;
    if (cmp_id(id, CPUID_VENDOR_QEMU))         return CPUID_VENDOR_QEMU;
}