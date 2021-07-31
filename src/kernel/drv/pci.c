#include "pci.h"

#include "../io.h"

pci_device devices[20];
int device_count = 0;

u32 pci_configRead(u8 bus, u8 device, u8 func, u8 offset) {
    u32 addr;
    u32 lbus  = (u32)bus;
    u32 ldevice = (u32)device;
    u32 lfunc = (u32)func;
 
    addr = (u32)((lbus << 16) | (ldevice << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((u32)0x80000000));
 
    outl(0xCF8, addr);
    return inl(0xCFC);
}

u16 pci_vendorID(u8 bus, u8 device, u8 func) {
    return LOW16(pci_configRead(bus, device, func, 0x00));
}

u16 pci_deviceID(u8 bus, u8 device, u8 func) {
    return HIGH16(pci_configRead(bus, device, func, 0x00));
}

u8 pci_baseClass(u8 bus, u8 device, u8 func) {
    return pci_configRead(bus, device, func, 0x08) >> 24;
}

u8 pci_subClass(u8 bus, u8 device, u8 func) {
    return (pci_configRead(bus, device, func, 0x08) >> 16) & 0xff;
}

u8 pci_progIf(u8 bus, u8 device, u8 func) {
    return (pci_configRead(bus, device, func, 0x08) >> 8) & 0xff;
}

u8 pci_revision(u8 bus, u8 device, u8 func) {
    return pci_configRead(bus, device, func, 0x08) & 0xff;
}

u8 pci_headerType(u8 bus, u8 device, u8 func) {
    return (pci_configRead(bus, device, func, 0x0C) >> 16) & 0xff;
}

u32 pci_BAR0(pci_device* d) {
    return pci_configRead(d->bus, d->device, d->function, 0x10);
}

u32 pci_BAR1(pci_device* d) {
    return pci_configRead(d->bus, d->device, d->function, 0x14);
}

u32 pci_BAR2(pci_device* d) {
    return pci_configRead(d->bus, d->device, d->function, 0x18);
}

u32 pci_BAR3(pci_device* d) {
    return pci_configRead(d->bus, d->device, d->function, 0x1C);
}

u32 pci_BAR4(pci_device* d) {
    return pci_configRead(d->bus, d->device, d->function, 0x20);
}

u32 pci_BAR5(pci_device* d) {
    return pci_configRead(d->bus, d->device, d->function, 0x24);
}

int pci_devicesFound() {
    return device_count;
}

pci_device* pci_getDevice(int i) {
    return &devices[i];
}

void pci_addDevice(u8 bus, u8 device, u8 func) {
    u16 venid = pci_vendorID(bus, device, func);
    if (venid == 0xffff) return;

    pci_device* d = &devices[device_count];

    d->bus       = bus;
    d->device    = device;
    d->function  = func;

    d->vendorID  = venid;
    d->deviceID  = pci_deviceID(bus, device, func);

    d->baseClass = pci_baseClass(bus, device, func);
    d->subClass  = pci_subClass(bus, device, func);
    d->progIf    = pci_progIf(bus, device, func);
    d->revision  = pci_revision(bus, device, func);

    device_count++;
}

void pci_checkDevice(u8 bus, u8 device) {
    u8 func = 0;

    u16 venid = pci_vendorID(bus, device, func);
    if (venid == 0xffff) return;

    pci_addDevice(bus, device, func);
    
    u8 headerType = pci_headerType(bus, device, func);
    if ( (headerType & 0x80) != 0 ) {
        for (func = 1; func < 8; func++)
            if (pci_vendorID(bus, device, func) != 0xffff)
                pci_addDevice(bus, device, func);
    }
}

void pci_checkBus(u8 bus) {
    for (int i = 0; i < 32; i++)
        pci_checkDevice(bus, i);
}

void pci_checkAll() {
    if ( (pci_headerType(0, 0, 0) & 0x80) == 0 ) {
        pci_checkBus(0);
    } else {
        for (int i = 0; i < 8; i++)
            pci_checkBus(i);
    }
}

void pci_init() {
    pci_checkAll();
}