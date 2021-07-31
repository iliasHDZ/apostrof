#pragma once

#include "../utils.h"

#include "pci_classes.h"
#include "pci_devices.h"

typedef struct pci_device {
    u8 bus;
    u8 device;
    u8 function;

    u16 vendorID;
    u16 deviceID;

    u8 baseClass;
    u8 subClass;
    u8 progIf;
    u8 revision;
} pci_device;

u32 pci_configRead(u8 bus, u8 device, u8 func, u8 offset);

u16 pci_vendorID(u8 bus, u8 device, u8 func);
u16 pci_deviceID(u8 bus, u8 device, u8 func);

u8 pci_baseClass(u8 bus, u8 device, u8 func);
u8 pci_subClass(u8 bus, u8 device, u8 func);
u8 pci_progIf(u8 bus, u8 device, u8 func);
u8 pci_revision(u8 bus, u8 device, u8 func);

u32 pci_BAR0(pci_device* d);

u32 pci_BAR1(pci_device* d);

u32 pci_BAR2(pci_device* d);

u32 pci_BAR3(pci_device* d);

u32 pci_BAR4(pci_device* d);

u32 pci_BAR5(pci_device* d);

int pci_devicesFound();
pci_device* pci_getDevice(int i);

void pci_init();