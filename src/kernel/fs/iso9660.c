#include "iso9660.h"

static u32 iso_lba_to_512(iso_voldesc_primary* pvd, u32 lba) {
    return pvd->log_block_size == 0 ? lba : (lba * pvd->log_block_size) / 0x200;
}

void iso_open(drive* device) {
    char buffer[512];
    iso_voldesc* vd = (iso_voldesc*) buffer;

    if (device->type == PARTITION)
        return;

    dbg_write("!!!");

    if (drive_read(device, 0x10 * 0x800, 1, buffer) != 0)
        return;

    dbg_hexDump(buffer, 0x200);

    if (!cmpstr(vd->std_id, "CD001"))
        return;

    if (vd->type != 1)
        return;

    if (vd->version != 1)
        return;

    iso_voldesc_primary* pvd = (iso_voldesc_primary*) vd;

    u32 pathtab_sectors = DIVIDE_CEIL(pvd->path_table_size, 0x200);
    u8* path_table = kmalloc(pathtab_sectors * 0x200);

    if (drive_read(device, iso_lba_to_512(pvd, pvd->path_table_lba), pathtab_sectors, path_table) != 0)
        return;

    dbg_hexDump(path_table, 0x200);

    /*int i = 0;
    while (i < pvd->path_table_size) {
        iso_pathtab_entry* entry = (iso_pathtab_entry*) &path_table[i];
        
    }*/
}