const fs = require('fs');

const enums = {
    'os_id u32': {
        0: 'Linux',
        1: 'GNU HURD',
        2: 'MASIX',
        3: 'FreeBSD',
        4: 'BSD'
    }
}

function getDataFromBuffer(buffer, type, offset) {
    let size = null;
    let data = null;

    switch (type) {
    case 'u8':
        data = buffer.readUInt8(offset);
        size = 1;
        break;
    case 'u16':
        data = buffer.readUInt16LE(offset);
        size = 2;
        break;
    case 'u32':
        data = buffer.readUInt32LE(offset);
        size = 4;
        break;
    case 'i8':
        data = buffer.readInt8(offset);
        size = 1;
        break;
    case 'i16':
        data = buffer.readInt16LE(offset);
        size = 2;
        break;
    case 'i32':
        data = buffer.readInt32LE(offset);
        size = 4;
        break;
    case 'utime32':
        data = new Date(buffer.readUInt32LE(offset) * 1000);
        size = 4;
        break;
    default:
        return null;
    }

    return [data, size];
}

function structToObject(buffer, struct, offset = 0) {
    let obj = {};

    for (let mem of struct) {
        let type = mem.split(' ')[0];
        let name = mem.split(' ')[1];
        let size = null;

        if (mem.split(' ').length > 2)
            size = mem.split(' ')[2];

        let elems = [];
        let data;

        for (let i = 0; i < (size == null ? 1 : size); i++) {
            let m = getDataFromBuffer(buffer, type, offset);

            if (!m) {
                if (type == 'pad')
                    offset += +name;
                else
                    for (let [key, value] of Object.entries(enums))
                        if (key.split(' ')[0] == type) {
                            m = getDataFromBuffer(buffer, key.split(' ')[1], offset);

                            data = value[m[0]];
                            offset += m[1];
                        }
            } else {
                data = m[0];
                offset += m[1];
            }

            elems.push(data);
        }

        if (data != null && name)
            obj[name] = size == null ? data : elems;
    }

    return obj;
}

let buffer = fs.readFileSync('test_disk.img');

const superblock_struct = [
    'u32 inode_count',
    'u32 block_count',
    'u32 blocks_for_super',
    'u32 unalloc_blocks',
    'u32 unalloc_inodes',
    'u32 superblock_num',
    'u32 log_block_size',
    'u32 log_frag_size',
    'u32 blocks_per_group',
    'u32 frags_per_group',
    'u32 inodes_per_group',
    'utime32 mtime',
    'utime32 wtime',
    'u16 mnt_count',
    'u16 max_mnt_count',
    'u16 magic',
    'u16 state',
    'u16 errors',
    'u16 minor_rev_level',
    'u32 lastcheck',
    'u32 checkinterval',
    'os_id creator_os',
    'u32 rev_level',
    'u16 def_resuid',
    'u16 def_resgid'
];

const blockgroup_struct = [
    'u32 block_bitmap',
    'u32 inode_bitmap',
    'u32 inode_table',
    'u16 free_blocks',
    'u16 free_inodes',
    'u16 used_dirs',
    'u16 pad',
    'pad 12'
];

const inode_struct = [
    'u16 mode',
    'u16 uid',
    'u32 size',
    'utime32 atime',
    'utime32 ctime',
    'utime32 mtime',
    'utime32 dtime',
    'u16 gid',
    'u16 nlinks',
    'u32 blocks',
    'u32 flags',
    'u32 osd1',
    'u32 block 15',
    'u32 generation',
    'u32 fileacl',
    'u32 diracl',
    'u32 faddr',
    'u32 osd2'
];

const dir_entry_struct = [
    'u32 inode',
    'u16 rec_len',
    'u8 name_len',
    'u8 file_type'
];

const superblock = structToObject(buffer, superblock_struct, 0x400);

let getBlockOffset = n => n * (0x400 << superblock.log_block_size);

let getInodeLocation = i => [Math.floor((i - 1) / superblock.inodes_per_group), (i - 1) % superblock.inodes_per_group];

const bdt_offset = getBlockOffset(1);

let getInode = i => {
    let [group, index] = getInodeLocation(i);

    let group_info = structToObject(buffer, blockgroup_struct, bdt_offset + group * 32);
    
    let inode_offset = getBlockOffset(group_info.inode_table) + index * 128;

    return structToObject(buffer, inode_struct, inode_offset);
}

const blockgroup0 = structToObject(buffer, blockgroup_struct, getBlockOffset(1));

console.log(superblock);
console.log(blockgroup0);

const root = getInode(2);

console.log(root);

let rootcon = getBlockOffset(root.block[0]);

let parseDirectory = data => {
    let offset = 0;
    let entries = [];

    while (offset < data.length) {
        if (data.readUInt32LE(offset) == 0)
            break;
        
        let entry = structToObject(data, dir_entry_struct, offset);
        let name = data.slice(offset + 8, offset + 8 + entry.name_len).toString('utf8');

        entries.push({...entry, name});
        offset += entry.rec_len;
    }

    return entries;
}

let rootdir = buffer.slice(rootcon, rootcon + (0x400 << superblock.log_block_size));

console.log(parseDirectory(rootdir));