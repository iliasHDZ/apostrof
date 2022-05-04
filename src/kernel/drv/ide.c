#include "ide.h"
#include "pci.h"

#include "../io.h"

#include "../vga.h"

#include "timer.h"
#include "drive.h"

extern u8 drive_addIDEDevice(char* model, u32 size, u8 atapi, u8 present, u8 channel,
                               u8 drv, u16 signature, u16 capabilities, u32 command_sets);

typedef struct {
    u16 base;
    u16 ctrl;
    u16 bmide;
    u8  nIEN;
} ide_channel_regs;

ide_channel_regs channels[2];

#define offset(t, d, o) *(t*)(d + o)

void ide_write(u8 ch, u8 reg, u8 data) {
    ide_channel_regs* chr = &channels[ch];

    if (reg > 0x07 && reg < 0x0C)
        ide_write(ch, ATA_REG_CONTROL, 0x80 | chr->nIEN);
    
    if (reg < 0x08)
        outb(chr->base  + reg - 0x00, data);
    else if (reg < 0x0C)
        outb(chr->base  + reg - 0x06, data);
    else if (reg < 0x0E)
        outb(chr->ctrl  + reg - 0x0A, data);
    else if (reg < 0x16)
        outb(chr->bmide + reg - 0x0E, data);
    
    if (reg > 0x07 && reg < 0x0C)
        ide_write(ch, ATA_REG_CONTROL, chr->nIEN);
}

u8 ide_read(u8 ch, u8 reg) {
    u8 res;
    if (reg > 0x07 && reg < 0x0C)
        ide_write(ch, ATA_REG_CONTROL, 0x80 | channels[ch].nIEN);

    if (reg < 0x08)
        res = inb(channels[ch].base  + reg - 0x00);
    else if (reg < 0x0C)
        res = inb(channels[ch].base  + reg - 0x06);
    else if (reg < 0x0E)
        res = inb(channels[ch].ctrl  + reg - 0x0A);
    else if (reg < 0x16)
        res = inb(channels[ch].bmide + reg - 0x0E);

    if (reg > 0x07 && reg < 0x0C)
        ide_write(ch, ATA_REG_CONTROL, channels[ch].nIEN);
    
    return res;
}

u32 ide_readl(u8 ch, u8 reg) {
    u32 res;
    if (reg > 0x07 && reg < 0x0C)
        ide_write(ch, ATA_REG_CONTROL, 0x80 | channels[ch].nIEN);

    if (reg < 0x08)
        res = inl(channels[ch].base  + reg - 0x00);
    else if (reg < 0x0C)
        res = inl(channels[ch].base  + reg - 0x06);
    else if (reg < 0x0E)
        res = inl(channels[ch].ctrl  + reg - 0x0A);
    else if (reg < 0x16)
        res = inl(channels[ch].bmide + reg - 0x0E);

    if (reg > 0x07 && reg < 0x0C)
        ide_write(ch, ATA_REG_CONTROL, channels[ch].nIEN);
    
    return res;
}

void ide_select_drive(u8 ch, u8 drv) {
    u8 hdsel = ide_read(ch, ATA_REG_HDDEVSEL);
    ide_write(ch, ATA_REG_HDDEVSEL, hdsel & ~(1 << 4) | (drv << 4));
}

void ide_lba_mode(u8 ch, u8 lba) {
    u8 hdsel = ide_read(ch, ATA_REG_HDDEVSEL);
    ide_write(ch, ATA_REG_HDDEVSEL, hdsel & ~(1 << 6) | (lba << 6));
}

u8 ide_set_lba(ide_drive* d, u32 lba, u8 sectornum) {
    u8 lba_mode, lba_io[6], head;

    if (lba >= 0x10000000) {
        lba_mode = 2;

        lba_io[0] = (lba & 0x000000FF) >> 0;
        lba_io[1] = (lba & 0x0000FF00) >> 8;
        lba_io[2] = (lba & 0x00FF0000) >> 16;
        lba_io[3] = (lba & 0xFF000000) >> 24;
        lba_io[4] = 0;
        lba_io[5] = 0;
        head      = 0;
    } else if (d->capabilities & 0x200) {
        lba_mode  = 1;

        lba_io[0] = (lba & 0x00000FF) >> 0;
        lba_io[1] = (lba & 0x000FF00) >> 8;
        lba_io[2] = (lba & 0x0FF0000) >> 16;
        lba_io[3] = 0;
        lba_io[4] = 0;
        lba_io[5] = 0;
        head      = (lba & 0xF000000) >> 24;
    } else if (d->capabilities & 0x200) {
        lba_mode  = 0;

        u8  sect  = (lba % 63) + 1;
        u16 cyl   = (lba + 1 - sect) / (16 * 63);
        u8  head  = (lba + 1  - sect) % (16 * 63) / (63);
        
        lba_io[0] = sect;
        lba_io[1] = (cyl >> 0) & 0xff;
        lba_io[2] = (cyl >> 8) & 0xff;
        lba_io[3] = 0;
        lba_io[4] = 0;
        lba_io[5] = 0;
    }

    ide_write(d->channel, ATA_REG_HDDEVSEL, 0xA0 | (d->drive << 4) | ((lba_mode > 0) << 6) | head);
 
    if (lba_mode == 2) {
      ide_write(d->channel, ATA_REG_SECCOUNT1,   0);
      ide_write(d->channel, ATA_REG_LBA3,   lba_io[3]);
      ide_write(d->channel, ATA_REG_LBA4,   lba_io[4]);
      ide_write(d->channel, ATA_REG_LBA5,   lba_io[5]);
   }

   ide_write(d->channel, ATA_REG_SECCOUNT0,   sectornum);
   ide_write(d->channel, ATA_REG_LBA0,   lba_io[0]);
   ide_write(d->channel, ATA_REG_LBA1,   lba_io[1]);
   ide_write(d->channel, ATA_REG_LBA2,   lba_io[2]);
 
    return lba_mode;
}

unsigned char ide_polling(unsigned char channel, unsigned int advanced_check) {
    for(int i = 0; i < 4; i++)
        ide_read(channel, ATA_REG_ALTSTATUS);
 
    while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY);
    
    if (advanced_check) {
        unsigned char state = ide_read(channel, ATA_REG_STATUS);

        if (state & ATA_SR_ERR)
            return 2;

        if (state & ATA_SR_DF)
            return 1;

        if ((state & ATA_SR_DRQ) == 0)
            return 3;
    
    }
    
    return 0;
}

void ide_addDevice(u8 present, u8 ch, u8 drv, u8 type, u8* id_data) {
    u16 signature    = offset(u16, id_data, ATA_IDENT_DEVICETYPE);
    u16 capabilities = offset(u16, id_data, ATA_IDENT_CAPABILITIES);
    u32 command_sets = offset(u32, id_data, ATA_IDENT_COMMANDSETS);
    u32 size;

    char model[40];

    if (command_sets & (1 << 26))
        size = offset(u32, id_data, ATA_IDENT_MAX_LBA_EXT);
    else
        size = offset(u32, id_data, ATA_IDENT_MAX_LBA);

    for (int i = 0; i < 40; i += 2) {
        model[i] = id_data[ATA_IDENT_MODEL + i + 1];
        model[i + 1] = id_data[ATA_IDENT_MODEL + i];
    }

    drive_addIDEDevice(
        model,
        size,
        type,
        present,
        ch,
        drv,
        signature,
        capabilities,
        command_sets
    );
}

void ide_search(pci_device* d) {
    u8 id_data[512];
    
    u32 BAR0 = pci_BAR0(d);
    u32 BAR1 = pci_BAR1(d);
    u32 BAR2 = pci_BAR2(d);
    u32 BAR3 = pci_BAR3(d);
    u32 BAR4 = pci_BAR4(d);

    channels[0] = (ide_channel_regs) {
        BAR0 ? BAR0 & ~(u32)0x03 : 0x1F0,
        BAR1 ? BAR1 & ~(u32)0x03 : 0x3F6,

        (BAR4 & ~(u32)0x03) + 0
    };

    channels[1] = (ide_channel_regs) {
        BAR2 ? BAR2 & ~(u32)0x03 : 0x170,
        BAR3 ? BAR3 & ~(u32)0x03 : 0x376,

        (BAR4 & ~(u32)0x03) + 8
    };

    ide_write(0, ATA_REG_CONTROL, 2);
    ide_write(1, ATA_REG_CONTROL, 2);

    for (int c = 0; c < 2; c++)
        for (int d = 0; d < 2; d++) {
            u8 err = 0, type = IDE_ATA;

            ide_write(c, ATA_REG_HDDEVSEL, 0xA0 | (d << 4));
            timer_sleep(1);

            ide_write(c, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
            timer_sleep(1);

            if (ide_read(c, ATA_REG_STATUS) == 0) continue;

            while (1) {
                u8 s = ide_read(c, ATA_REG_STATUS);
                
                if (s & ATA_SR_ERR) {
                    err = 1;
                    break;
                }

                if (!(s & ATA_SR_BSY) && (s & ATA_SR_DRQ)) break;
            }

            if (err != 0) {
                u8 cl = ide_read(c, ATA_REG_LBA1);
                u8 ch = ide_read(c, ATA_REG_LBA2);

                if (cl == 0x14 && ch == 0xEB)
                    type = IDE_ATAPI;
                else if (cl == 0x69 && ch == 0x96)
                    type = IDE_ATAPI;
                else
                    continue;

                ide_write(c, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
                timer_sleep(1);
            }

            for (int i = 0; i < 128; i++)
                ((u32*)id_data)[i] = ide_readl(c, ATA_REG_DATA);

            ide_addDevice(1, c, d, type, id_data);
        }
}

i8 ide_ata_read(ide_drive* d, u32 sector, u8 sectornum, u8* dst) {
    u8 ch = d->channel, cmd, err;

    ide_write(ch, ATA_REG_CONTROL, channels[ch].nIEN = 0x0 + 0x02);

    while (ide_read(ch, ATA_REG_STATUS) & ATA_SR_BSY);

    u8 lba_mode = ide_set_lba(d, sector, sectornum);

    if (lba_mode == 2) cmd = ATA_CMD_READ_PIO_EXT;
    else               cmd = ATA_CMD_READ_PIO;

    ide_write(ch, ATA_REG_COMMAND, cmd);

    for (int s = 0; s < sectornum; s++) {
        if (err = ide_polling(ch, 1))
            return err;

        for (int i = 0; i < 256; i++)
            ((u16*)dst)[s * 256 + i] = inw(channels[ch].base);
    }
    
    return 0;
}

void ide_init() {
    for (int i = 0; i < pci_devicesFound(); i++) {
        pci_device* d = pci_getDevice(i);

        if (d->baseClass == PCI_MASS_STORAGE && d->baseClass == 0x01) {
            ide_search(d);
            break;
        }
    }
}