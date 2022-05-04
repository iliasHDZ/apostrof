#pragma once

#include "drive.h"
#include "../kmm.h"

typedef struct {
    char signature[8];
    u32  revision;
    u32  header_size;
    u32  crc32;
    u32  reserved;
    u64  this_lba;
    u64  backup_lba;
    u64  first_lba;
    u64  last_lba;
    u8   disk_guid[16];
    u64  gpt_lba;
    u32  pe_count;
    u32  pe_size;
    u32  crc32_pe;
} __attribute__((packed)) gpt_header;

typedef struct {
    u8   type_guid[16];
    u8   unique_guid[16];
    u64  start_lba;
    u64  end_lba;
    u64  attributes;
    // Name present here as UNICODE16-LE
} __attribute__((packed)) gpt_entry;

u8 gpt_read(drive* d);