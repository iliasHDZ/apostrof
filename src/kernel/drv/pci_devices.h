#pragma once

#include "../utils.h"

#define PCI_VENDOR_INTEL  0x8086
#define PCI_VENDOR_CIRRUS 0x1013

const char* pci_vendorName(u16 venid);