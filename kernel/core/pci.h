#ifndef _PCI_H_
#define _PCI_H_

#include "types.h"

// https://wiki.osdev.org/PCI
#define PCI_CONFIG_ADDRESS_PORT 0xcf8
#define PCI_CONFIG_DATA_PORT	0xcfc

#define PCI_VENDOR_ID_OFFSET 0x00
#define PCI_DEVICE_ID_OFFSET 0x02
#define PCI_COMMAND_OFFSET   0x04
#define PCI_BAR0_OFFSET      0x10
#define PCI_INTR_LINE_OFFSET 0x3c

struct pci_device {
	uint8_t bus;
	uint8_t slot;
	uint16_t vendor;
	uint16_t device;
	uint16_t bar0;
	uint8_t irq;
};

void pci_enable_bus_master(struct pci_device *dev);
int pci_find_device(struct pci_device *dev, uint16_t target_vendor, uint16_t target_device);

#endif
