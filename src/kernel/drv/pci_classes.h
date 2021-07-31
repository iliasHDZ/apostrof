#pragma once

#include "../utils.h"

#define PCI_UNCLASSIFIED  0x00
#define PCI_MASS_STORAGE  0x01
#define PCI_NETWORK       0x02
#define PCI_DISPLAY       0x03
#define PCI_MULTIMEDIA    0x04
#define PCI_MEMORY        0x05
#define PCI_BRIDGE        0x06
#define PCI_COMMUNICATION 0x07
#define PCI_PERIPHERAL    0x08
#define PCI_INPUT         0x09
#define PCI_DOCKING       0x0A
#define PCI_PROCESSOR     0x0B

struct pci_device;

const char* pci_deviceClassName(struct pci_device* dev);