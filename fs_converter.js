const fs   = require('fs');
const path = require('path');

function newFile(parent, name, content) {
    return {
        dir:           false,
        parent:        parent,
        name:          name,
        creation_date: new Date().getTime(),
        modified_date: new Date().getTime(),
        content:      content
    }
}

function newDirectory(parent, name, children = []) {
    return {
        dir:           true,
        parent:        parent,
        name:          name,
        creation_date: new Date().getTime(),
        modified_date: new Date().getTime(),
        content:      children
    }
}

function encodeDirToObject(object, dir_id, dir_path) {
    let files = fs.readdirSync(dir_path);

    for (let file of files) {
        let file_path = path.join(dir_path, file);

        if (fs.lstatSync(file_path).isDirectory()) {
            let directory = newDirectory(dir_id, file);
            let id        = object.push(directory) - 1;
            
            object[dir_id].content.push(id);
            encodeDirToObject(object, id, file_path);
        } else {
            let fileo = newFile(dir_id, file, fs.readFileSync(file_path));
            let id    = object.push(fileo) - 1;

            object[dir_id].content.push(id);
        }
    }
}

function encodeAllToObject(path) {
    let ret = [
        null,
        newDirectory(0, "")
    ];

    encodeDirToObject(ret, 1, path);
    return ret;
}

function isPresent(bitmap, file) {
    let byte = file / 8;
    let bit  = file % 8;

    return (bitmap[byte] >> bit) & 1;
}

function setPresent(bitmap, file, present) {
    let byte = Math.floor(file / 8);
    let bit  = file % 8;

    if (present)
        bitmap[byte] |= 1 << bit;
    else
        bitmap[byte] &= ~(1 << bit);
}

function applyObject(buffer, object) {
    let fs_string = buffer.slice(2, 10).toString();
    if (fs_string != "APOFS   ") return null;

    let fs_base = Math.ceil(buffer.length / 4096) * 4096;
    buffer = Buffer.concat([buffer, Buffer.alloc(fs_base - buffer.length)]);

    let bitmap_sectors = Math.ceil((object.length - 1) / (8 * 512));
    let bitmap = Buffer.alloc(bitmap_sectors * 512);

    let children_max = 0;

    for (let f of object)
        if (f && f.dir)
            children_max = Math.max(children_max, f.content.length);

    let desc_size      = Math.ceil((4 * children_max + 0x58) / 512);

    let table          = Buffer.alloc(desc_size * object.length * 512);
    let file_contents  = Buffer.alloc(0);

    for (let i = 1; i < object.length; i++) {
        let f = object[i];

        let pos = (i - 1) * desc_size * 512;

        table.writeUInt8(f.dir ? 1 : 2, pos);
        table.writeUInt32LE(f.parent, 1 + pos);
        table.write(f.name, 5 + pos, 59);
        
        //descriptor.writeBigUInt64LE(0, 64);//f.creation_date, 64);
        //descriptor.writeBigUInt64LE(0, 74);//f.modified_date, 72);
        
        table.writeUInt16LE(0, 80 + pos);

        let offset = 0x58;

        if (f.dir)
            for (let child of f.content) {
                table.writeUInt32LE(child, offset + pos);
                offset += 4;
            }
        else {
            let sectors  = Math.ceil(f.content.length / 512);
            let data_loc = (buffer.length + bitmap.length + table.length + file_contents.length) / 512;

            table.writeUInt32LE(data_loc, offset + pos);
            table.writeUInt32LE(data_loc + sectors, offset + 4 + pos);

            table.writeUInt32LE(f.content.length - (sectors - 1) * 512, offset + 12 + pos);

            file_contents = Buffer.concat([file_contents, f.content, Buffer.alloc(sectors * 512 - f.content.length)]);
        }

        setPresent(bitmap, i, 1);
    }

    setPresent(bitmap, 0, 1);

    buffer.writeUInt32LE(buffer.length / 512, 14);
    buffer = Buffer.concat([buffer, bitmap]);
    buffer.writeUInt32LE(buffer.length / 512, 18);
    buffer = Buffer.concat([buffer, table]);
    buffer.writeUInt32LE(buffer.length / 512, 22);
    buffer = Buffer.concat([buffer, file_contents]);
    buffer.writeUInt32LE(desc_size, 26);

    return buffer;
}

let obj = encodeAllToObject("./fs");

let img_content = fs.readFileSync("./dist/rom_file.bin");

fs.writeFileSync("./dist/apostrof.img", applyObject(img_content, obj));