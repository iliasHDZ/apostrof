#include "gpt.h"

static const char* gpt_signature = "EFI PART";

static u8 guid_iszero(u8* guid) {
    for (int i = 0; i < 16; i++)
        if (guid[i] != 0)
            return 0;
    
    return 1;
}

u8 gpt_read(drive* d) {
    u8 header_sector[512];
    gpt_header* header = (gpt_header*)header_sector;

    if (drive_read(d, 1, 1, &header_sector) != 0)
        return 0;

    for (int i = 0; i < 8; i++)
        if (header->signature[i] != gpt_signature[i])
            return 0;

    u32 entries_start = header->gpt_lba;
    u32 entries_count = header->pe_count;
    u32 entry_size    = header->pe_size;

    u32 gpt_sectors = DIVIDE_CEIL(entries_count * entry_size, 0x200);

    u32 max_name_size = entry_size - 0x38;

    u8* entries = kmalloc(gpt_sectors * 0x200);

    if (drive_read(d, entries_start, gpt_sectors, entries) != 0) {
        kfree(entries);
        return 0;
    }

    for (int i = 0; i < entries_count; i++) {
        gpt_entry* entry = (gpt_entry*)(entries + i * entry_size);

        if (guid_iszero(entry->type_guid))
            continue;

        u16* wide_name = (u8*)entry + 0x38;
        u32  name_len  = 0;

        while (wide_name[name_len] != 0 && (name_len * 2) < max_name_size)
            name_len++;

        char* name = kmalloc(name_len + 1);

        for (int j = 0; j < name_len; j++)
            name[j] = wide_name[j] & 0xff;

        name[name_len] = 0;

        drive_addPartition(name, d, entry->start_lba, entry->end_lba);
    }

    kfree(entries);
    return 1;
}