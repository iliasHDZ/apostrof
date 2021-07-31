#include "pci_devices.h"

const char* pci_vendorName(u16 venid) {
    switch (venid) {
        case PCI_VENDOR_INTEL:  return "Intel Corporation";
        case PCI_VENDOR_CIRRUS: return "Cirrus Logic";
        default:                return 0;
    }
}