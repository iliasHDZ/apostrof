#include "elf32.h"
#include "../kmm.h"

#include "../vga.h"

#define SHT_NULL     0
#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_RELA     4
#define SHT_HASH     5
#define SHT_DYNAMIC  6
#define SHT_NOTE     7
#define SHT_NOBITS   8
#define SHT_REL      9
#define SHT_SHLIB    10
#define SHT_DYNSYM   11
#define SHT_LOPROC   0x70000000
#define SHT_HIPROC   0x7fffffff
#define SHT_LOUSER   0x80000000
#define SHT_HIUSER   0xffffffff

#define SHF_WRITE     0x1
#define SHF_ALLOC     0x2
#define SHF_EXECINSTR 0x4
#define SHF_MASKPROC  0xf0000000

#define DIVIDE_CEIL(a, b) a / b + (a % b != 0)

int elf32_load(apo_fs* fs, const char* path, task* task, u32 stack_size, char** error) {
    if (fs == 0 || path == 0) return 0;

    u32 file_id = apofs_getFile(fs, path);
    if (file_id == 0) {
        *error = "Cannot find file";
        return 0;
    }

    u32 file_size = apofs_getFileSize(fs, file_id);
    if (file_size == 0) {
        *error = "Cannot open file";
        return 0;
    }

    u8* buffer = (u8*)kmalloc(file_size);

    if (apofs_read(fs, file_id, buffer, 0, file_size) < file_size) {
        kfree(buffer);
        *error = "Cannot read file";
        return 0;
    }

    elf32_hdr* header = (elf32_hdr*)buffer;
    
    if (*(u32*)&(header->e_ident) != (u32)0x464C457F) {
        kfree(buffer);
        *error = "Not an ELF file";
        return 0;
    }

    if (header->e_ident[4] != 1 || header->e_ident[5] != 1) {
        kfree(buffer);
        *error = "Cannot read this specific ELF file specification";
        return 0;
    }

    if (header->e_type != 2) {
        kfree(buffer);
        *error = "Not an executable file";
        return 0;
    }

    if (header->e_machine != 3) {
        kfree(buffer);
        *error = "File must be elf_i386";
        return 0;
    }

    vmem* mem = vmem_cloneKernelMemory();
    if (mem == 0) return 0;

    vmem_switchMemory(mem);

    elf32_shdr* sections = (elf32_shdr*)(buffer + header->e_shoff);
    u32 section_number   = header->e_shnum;
    u32 sectionhdr_size  = header->e_shentsize;

    const char* string_table = 0;

    if (header->e_shstrndx != 0) {
        elf32_shdr* strsec = (elf32_shdr*)((u32)sections + header->e_shstrndx * sectionhdr_size);

        if (strsec->sh_type == SHT_STRTAB)
            string_table = (const char*)(buffer + strsec->sh_offset);
    }

    u32 heap_address = 0;

    for (int i = 0; i < section_number; i++) {
        elf32_shdr* section = (elf32_shdr*)((u32)sections + i * sectionhdr_size);
        if (section->sh_type == SHT_NULL) continue;

        const char* name = "";
        if (string_table != 0)
            name = string_table + section->sh_name;

        u8 flag_write = section->sh_flags & SHF_WRITE;
        u8 flag_alloc = section->sh_flags & SHF_ALLOC;
        u8 flag_exec  = section->sh_flags & SHF_EXECINSTR;

        if (flag_alloc && section->sh_size > 0) {
            u32 paddr  = (section->sh_addr) >> 12;
            u32 plimit = (section->sh_addr + section->sh_size) >> 12;

            if (section->sh_addr + section->sh_size > heap_address)
                heap_address = section->sh_addr + section->sh_size;

            if (paddr == plimit)
                plimit++;

            for (u32 j = paddr; j < plimit; j++) {
                vmem_page* page = vmem_getPage(mem, j << 12);

                if (page != 0) {
                    if (!page->user) {
                        vmem_switchMemory(kernel_memory);
                        vmem_freeMemory(mem);
                        kfree(buffer);
                        *error = "Cannot allocate memory in kernel space";
                        return 0;
                    }

                    if (flag_write)
                        page->writable = 1;
                    continue;
                }

                vmem_allocPage(mem, j << 12, (flag_write ? VMEM_WRITABLE : 0) | VMEM_PRESENT | VMEM_USER);
            }
            
            memset(section->sh_addr, 0, section->sh_size);

            if (section->sh_type != SHT_NOBITS)
                memcpy(section->sh_addr, buffer + section->sh_offset, section->sh_size);
        }
    }

    u32 stack_pages = DIVIDE_CEIL(stack_size, 4096);
    u32 stack_base_page = VMEM_TASK_STACK >> 12;
    
    for (int i = 0; i < stack_pages; i++)
        vmem_allocPage(mem, (stack_base_page - i) << 12, VMEM_PRESENT | VMEM_WRITABLE | VMEM_USER);

    task->mem     = mem;

    task->entry   = header->e_entry;
    task->stack   = VMEM_TASK_STACK;
    task->heap    = heap_address;

    task->heap_limit = task->heap;

    kfree(buffer);
    return 1;
}