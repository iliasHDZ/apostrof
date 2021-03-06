#include "shell.hpp"

extern "C" {
#include "../drv/keyboard.h"
#include "../drv/keys.h"
#include "../drv/pci.h"
#include "../drv/drive.h"

#include "../vga.h"
#include "../io.h"

#include "../cpu/cpuid.h"

#include "../kmm.h"
}

#include "../fs/iso9660.hpp"

using namespace apo;

int max_input = 127;

char input_buffer[128];

u8 is_reading = 0;
int cursor = 0;

u8 input_x = 0;
u8 input_y = 0;

void render_input() {
    vga_setColor(VGA_LIGHT_GRAY, VGA_BLACK);
    
    vga_setCursor(input_x, input_y);

    vga_writeText(input_buffer);
    vga_writeChar(' ');

    vga_setCursor(input_x + cursor, input_y);
}

int get_input_length() {
    int i = 0;
    while (input_buffer[i] != 0) i++;

    return i;
}

void shell_start() {
    vga_setColor(VGA_LIGHT_GRAY, VGA_BLACK);
    vga_writeText("> ");

    is_reading = 1;
    cursor     = 0;

    input_x = vga_getCursorX();
    input_y = vga_getCursorY();

    for (int i = 0; i < max_input; i++)
        input_buffer[i] = 0;
}

struct arguments {
    int    argc;
    char** argv;
};

struct arguments parse_command() {
    u8 non_space = 0;
    int i = 0;

    int argc = 0;

    while (input_buffer[i] != 0) {
        char c = input_buffer[i];

        if (non_space && c == ' ') {
            argc++;
            input_buffer[i] = 0;
        }

        non_space = c != ' ';
        i++;
    }

    if (non_space) argc++;

    if (argc <= 0) return (struct arguments){0, 0};

    char** argv = (char**)kmalloc(sizeof(char*) * argc);

    non_space = 0;

    int arg = 0;

    for (int j = 0; j < i; j++) {
        char c = input_buffer[j];
        u8 nsp = c != ' ' && c != 0;

        if (!non_space && nsp) {
            argv[arg] = input_buffer + j;
            arg++;
        }

        non_space = nsp;
    }

    return (struct arguments){argc, argv};
}

u8 get_hex_value(char hex) {
    if (hex >= '0' && hex <= '9') return hex - '0';
    if (hex >= 'A' && hex <= 'F') return (hex - 'A') + 0xa;
    if (hex >= 'a' && hex <= 'f') return (hex - 'a') + 0xa;

    return 0xff;
}

u32 parse_hex(const char* hex) {
    u32 ret = 0;

    int i = 0;
    while (hex[i] != 0) {
        ret <<= 4;
        
        u8 v = get_hex_value(hex[i]);
        if (v == 0xff) return 0xffffffff;

        ret |= v;
        i++;
    }

    return ret;
}

void writeSize(u64 bytes) {
    if (bytes > 1024) {
        bytes >>= 10;

        if (bytes > 1024) {
            bytes >>= 10;

            if (bytes > 1024) {
                bytes >>= 10;

                vga_writeInteger(bytes);
                vga_writeText(" GiB");
                return;
            }

            vga_writeInteger(bytes);
            vga_writeText(" MiB");
            return;
        }

        vga_writeInteger(bytes);
        vga_writeText(" KiB");
        return;
    }

    vga_writeInteger(bytes);
    vga_writeText(" b");
}

void run_command(int argc, char** argv) {
    is_reading = 0;
    vga_writeChar('\n');
    
    if (argc < 1) {
        shell_start();
        return;
    }

    const char* command = argv[0];

    if (cmpstr(command, "cpuid")) {
        if (cpuid_check()) {
            vga_writeText("Processor Vendor ID: ");
            vga_print(cpuid_vendor());
        } else
            vga_print("CPUID is not supported");
    } else if (cmpstr(command, "shutdown")) {
        outw(0xB004, 0x2000);
        outw(0x604,  0x2000);
        outw(0x4004, 0x3400);
    } else if (cmpstr(command, "outb")) {
        if (argc == 3) {
            u32 port = parse_hex(argv[1]);
            u32 val  = parse_hex(argv[2]);

            if (port <= 0xffff && val <= 0xff)
                outb(port, val);
            else
                vga_print("invalid values");
        } else
            vga_print("too few arguments");
    } else if (cmpstr(command, "outw")) {
        if (argc == 3) {
            u32 port = parse_hex(argv[1]);
            u32 val  = parse_hex(argv[2]);

            if (port <= 0xffff && val <= 0xffff)
                outw(port, val);
            else
                vga_print("invalid values");
        } else
            vga_print("too few arguments");
    } else if (cmpstr(command, "outl")) {
        if (argc == 3) {
            u32 port = parse_hex(argv[1]);
            u32 val  = parse_hex(argv[2]);

            if (port <= 0xffff && val <= 0xffffffff)
                outl(port, val);
            else
                vga_print("invalid values");
        } else
            vga_print("too few arguments");
    } else if (cmpstr(command, "inb")) {
        if (argc == 2) {
            u32 port = parse_hex(argv[1]);

            if (port <= 0xffff) {
                vga_writeByte(inb(port));
                vga_writeChar('\n');
            } else
                vga_print("invalid port");
        } else
            vga_print("too few arguments");
    } else if (cmpstr(command, "inw")) {
        if (argc == 2) {
            u32 port = parse_hex(argv[1]);

            if (port <= 0xffff) {
                vga_writeWord(inw(port));
                vga_writeChar('\n');
            } else
                vga_print("invalid port");
        } else
            vga_print("too few arguments");
    } else if (cmpstr(command, "inl")) {
        if (argc == 2) {
            u32 port = parse_hex(argv[1]);

            if (port <= 0xffffffff) {
                vga_writeDWord(inl(port));
                vga_writeChar('\n');
            } else
                vga_print("invalid port");
        } else
            vga_print("too few arguments");
    } else if (cmpstr(command, "pci")) {
        if (argc == 4) {
            u32 bus = parse_hex(argv[1]);
            u32 dev = parse_hex(argv[2]);
            u32 fun = parse_hex(argv[3]);

            u32 dat = pci_configRead(bus, dev, fun, 0);

            u16 venid = dat & 0xffff;
            u16 devid = dat >> 16;

            if (venid == 0xffff)
                vga_print("No device found");
            else {
                vga_writeText("Vendor ID: ");
                vga_writeWord(venid);
                vga_writeChar('\n');
                
                vga_writeText("Device ID: ");
                vga_writeWord(devid);
                vga_writeChar('\n');
            }
        } else if (argc == 1) {
            int device_count = pci_devicesFound();

            vga_setColor(VGA_WHITE, VGA_BLACK);
            vga_print("PCI devices found:\n");

            for (int i = 0; i < device_count; i++) {
                pci_device* d = pci_getDevice(i);

                const char* vendor = pci_vendorName(d->vendorID);

                vga_setColor(VGA_WHITE, VGA_BLACK);
                if (vendor != 0)
                    vga_writeText(vendor);
                else {
                    vga_writeText("Vendor ");
                    vga_writeWord(d->vendorID);
                }

                vga_writeText(", Device ");
                vga_writeWord(d->deviceID);
                
                vga_setColor(VGA_DARK_GRAY, VGA_BLACK);
                vga_writeText(" (");
                vga_writeByte(d->bus);
                vga_writeText(", ");
                vga_writeByte(d->device);
                vga_writeText(", ");
                vga_writeByte(d->function);
                vga_writeText(")");

                vga_setColor(VGA_LIGHT_GRAY, VGA_BLACK);
                vga_writeChar('\n');
                vga_writeText("Device Type: ");
                vga_writeText(pci_deviceClassName(d));

                vga_setColor(VGA_DARK_GRAY, VGA_BLACK);
                vga_writeText(" (base: ");
                vga_writeByte(d->baseClass);
                vga_writeText(", sub: ");
                vga_writeByte(d->subClass);
                vga_writeText(", pi: ");
                vga_writeByte(d->progIf);
                vga_writeText(")\n\n");
            }
        } else
            vga_print("too few arguments");
    } else if (cmpstr(command, "drive")) {
        vga_print("Drives found:\n");

        drive* d;
        for (int i = 0; (d = drive_getDevice(i)) != 0; i++) {
            vga_setColor(VGA_WHITE, VGA_BLACK);

            vga_writeText(drive_path(d));

            vga_setColor(VGA_LIGHT_GRAY, VGA_BLACK);

            vga_writeText(" : ");

            if (d->type == DRIVE_IDE) {
                ide_drive* ide = (ide_drive*)d;

                vga_writeText("IDE ");

                vga_writeText(ide->channel ? "Secondary" : "Primary");
                vga_writeChar(' ');
                vga_writeText(ide->drive ? "Slave" : "Master");
                
                vga_writeText("\nModel: ");
                vga_writeText(ide->device_name);
            }

            vga_writeText("\nSize: ");
            writeSize(d->size * 512);
            vga_writeText("\n\n");
        }
    } else if (cmpstr(command, "partition")) {
        vga_print("Partitions found:\n");

        drive_partition* p;
        for (int i = 0; (p = drive_getPartitionByIndex(i)) != 0; i++) {
            vga_setColor(VGA_WHITE, VGA_BLACK);

            vga_writeText(p->device_name);

            vga_setColor(VGA_LIGHT_GRAY, VGA_BLACK);
            
            vga_writeText("\nSize: ");
            writeSize(p->size * 512);
            vga_writeText("\n\n");
        }
    } else if (cmpstr(command, "drive_read")) {
        if (argc >= 3) {
            u32 drv  = parse_hex(argv[1]);
            u32 sec  = parse_hex(argv[2]);
            u32 size = 512;

            if (argc >= 4)
                size  = parse_hex(argv[3]);

            drive* d = drive_getDevice(drv);

            if (d != 0) {
                u8* buffer = (u8*)kmalloc(512);

                u8 err;

                if ((err = drive_read(d, sec, 1, buffer)) != 0) {
                    vga_writeText("read error ");
                    vga_writeByte(err);
                    vga_writeChar('\n');
                } else
                    vga_hexDump(buffer, size);

                kfree(buffer);
            } else
                vga_print("drive doesn't exist");
        } else
            vga_print("too few arguments");
    } else if (cmpstr(command, "mount")) {
        u32 drv = parse_hex(argv[1]);

        drive* d = drive_getDevice(drv);
        auto fs = Filesystem::mount(d);
    } else {
        vga_writeText("unknown command: ");
        vga_print(command);
    }

    shell_start();
}

static void char_callback(char c) {
    if (!is_reading) return;

    for (int i = max_input - 1; i >= cursor + 1; i--)
        input_buffer[i] = input_buffer[i - 1];

    input_buffer[cursor] = c;
    
    cursor++;
    if (cursor >= max_input) cursor = max_input - 1;

    render_input();
}

static void keydown_callback(u16 scancode) {
    if (!is_reading) return;

    if (scancode == KEY_LEFT) {
        cursor--;
        if (cursor < 0) cursor = 0;

        vga_setCursor(input_x + cursor, input_y);
    } else if (scancode == KEY_RIGHT) {
        cursor++;

        int len = get_input_length();
        if (cursor >= len + 1) cursor = len;

        vga_setCursor(input_x + cursor, input_y);
    } else if (scancode == KEY_BACKSPACE) {
        if (cursor == 0) return;

        for (int i = cursor; i <= max_input; i++)
            input_buffer[i - 1] = input_buffer[i];

        cursor--;
        if (cursor < 0) cursor = 0;

        render_input();
    } else if (scancode == KEY_ENTER) {
        struct arguments a = parse_command();
        run_command(a.argc, a.argv);

        kfree(a.argv);
    }
}

void shell_init() {
    shell_start();
    input_buffer[max_input] = 0;

    keyboard_char(char_callback);
    keyboard_keydown(keydown_callback);

    render_input();
}