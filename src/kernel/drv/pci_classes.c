#include "pci_classes.h"

#include "pci.h"

const char* pci_class_unclassified[] = {
    "Unclassified"
};

const char* pci_class_mass_storage[] = {
    "SCSI storage controller",
    "IDE interface",
    "Floppy disk controller",
    "IPI bus controller",
    "RAID bus controller",
    "ATA controller",
    "SATA controller",
    "Serial Attached SCSI controller",
    "Non-Volatile memory controller"
};

const char* pci_class_network[] = {
    "Ethernet controller",
    "Token ring network controller",
    "FDDI network controller",
    "ATM network controller",
    "ISDN controller",
    "WorldFip controller",
    "PICMG controller",
    "Infiniband controller",
    "Fabric controller"
};

const char* pci_class_display[] = {
    "VGA compatible controller",
    "XGA compatible controller",
    "3D controller"
};

const char* pci_class_multimedia[] = {
    "Multimedia video controller",
    "Multimedia audio controller",
    "Computer telephony device",
    "Audio device"
};

const char* pci_class_memory[] = {
    "RAM memory",
    "FLASH memory"
};

const char* pci_class_bridge[] = {
    "Host bridge",
    "ISA bridge",
    "EISA bridge",
    "MicroChannel bridge",
    "PCI bridge",
    "PCMCIA bridge",
    "NuBus bridge",
    "CardBus bridge",
    "RACEway bridge",
    "Semi-transparent PCI-to-PCI bridge",
    "InfiniBand to PCI host bridge"
};

const char* pci_class_communication[] = {
    "Serial controller",
    "Parallel controller",
    "Multiport serial controller",
    "Modem",
    "GPIB controller",
    "Smard Card controller"
};

const char* pci_class_peripheral[] = {
    "PIC",
    "DMA controller",
    "Timer",
    "RTC",
    "PCI Hot-plug controller",
    "SD Host controller",
    "IOMMU"
};

const char* pci_class_input[] = {
    "Keyboard controller",
    "Digitizer Pen",
    "Mouse controller",
    "Scanner controller",
    "Gameport controller"
};

const char* pci_class_docking[] = {
    "Generic Docking Station"
};

const char* pci_class_processor[] = {
    "386",
    "486",
    "Pentium"
};

const char* pci_classes[] = {
    "Unclassified device",
    "Mass storage controller",
    "Network controller",
    "Display controller",
    "Multimedia controller",
    "Memory controller",
    "Bridge",
    "Communication controller",
    "Generic system peripheral",
    "Input device controller",
    "Docking station",
    "Processor"
};

const char* pci_deviceClassName(struct pci_device* dev) {
    const char** classnames = 0;

    if (dev->subClass == 0x80) {
        if (dev->baseClass > 0x0b) return "Unknown type";
        return pci_classes[dev->baseClass];
    }

    switch (dev->baseClass)
    {
    case PCI_UNCLASSIFIED:  return pci_class_unclassified[0];
    case PCI_MASS_STORAGE:  classnames = pci_class_mass_storage; break;
    case PCI_NETWORK:       classnames = pci_class_network; break;
    case PCI_DISPLAY:       classnames = pci_class_display; break;
    case PCI_MULTIMEDIA:    classnames = pci_class_multimedia; break;
    case PCI_MEMORY:        classnames = pci_class_memory; break;
    case PCI_BRIDGE:        classnames = pci_class_bridge; break;
    case PCI_COMMUNICATION: classnames = pci_class_communication; break;
    case PCI_PERIPHERAL:
        if (dev->subClass == 0x99) return "Timing card";
        classnames = pci_class_peripheral;
        break;
    case PCI_INPUT:         classnames = pci_class_input; break;
    case PCI_DOCKING:       classnames = pci_class_docking; break;
    case PCI_PROCESSOR:
        if (dev->subClass == 0x10) return "Alpha";
        if (dev->subClass == 0x20) return "Power PC";
        if (dev->subClass == 0x30) return "MIPS";
        if (dev->subClass == 0x40) return "Co-processor";

        classnames = pci_class_processor;
        break;
    default:                return "Unknown type";
    }

    return classnames[dev->subClass];
}