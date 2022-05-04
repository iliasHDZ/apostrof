#include "../utils.h"
#include "../drv/drive.h"

#define LE_BE_U16(N) \
    u16 N; \
    u16 MSB_##N;

#define LE_BE_U32(N) \
    u32 N; \
    u32 MSB_##N;

typedef struct {
    drive* device;
} iso_fs;

typedef struct {
    u8 type;
    char std_id[5];
    u8 version;
} iso_voldesc;

typedef struct {
    u8 type;
    char std_id[5];
    u8 version;
    u8 __1;
    char sys_id[32];
    char vol_id[32];
    u8 __2[8];
    LE_BE_U32(vol_space_size);
    u8 __3[32];
    LE_BE_U16(vol_set_size);
    LE_BE_U16(vol_seq_num);
    LE_BE_U16(log_block_size);
    LE_BE_U32(path_table_size);
    u32 path_table_lba;
} iso_voldesc_primary;

typedef struct {
    u8 di_size;
    u8 di_ext_size;
    u32 table_lba;
    u16 parent;
} iso_pathtab_entry;

void iso_open(drive* device);