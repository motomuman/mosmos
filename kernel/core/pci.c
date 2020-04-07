#include <stdint.h>
#include "print.h"
#include "nasmfunc.h"
#include "pci.h"

/*
 * https://wiki.osdev.org/PCI
 *
 *        31    | 30 - 24  | 23-16 | 15-11 | 10-8 |  7-0   |
 * +------------+----------+-------+-------+------+--------+
 * | Enable Bit | Reserved |  Bus  |  Dev  | Func | Offset |
 * +------------+----------+-------+-------+------+--------+
 */
static uint32_t read32(uint8_t bus, uint8_t slot, uint16_t offset) {
	uint32_t addr = (1UL << 31) | (bus << 16) | (slot << 11) | offset;
	io_out32(PCI_CONFIG_ADDRESS_PORT, addr);
	return io_in32(PCI_CONFIG_DATA_PORT);
}

static uint8_t read8(uint8_t bus, uint8_t slot, uint16_t offset) {
	uint32_t value = read32(bus, slot, offset & 0xfffc);
	return (value >> ((offset & 0x03) * 8)) & 0xff;
}

static uint16_t read16(uint8_t bus, uint8_t slot, uint16_t offset) {
	uint32_t value = read32(bus, slot, offset & 0xfffc);
	return (value >> ((offset & 0x03) * 8)) & 0xffff;
}

static void write32(uint8_t bus, uint8_t slot, uint16_t offset,
		uint32_t value) {
	uint32_t addr = (1UL << 31) | (bus << 16) | (slot << 11) | offset;
	io_out32(PCI_CONFIG_ADDRESS_PORT, addr);
	io_out32(PCI_CONFIG_DATA_PORT, value);
}

void pci_enable_bus_master(struct pci_device *dev) {
	uint32_t value = read32(dev->bus, dev->slot, PCI_COMMAND_OFFSET) | (1 << 2);
	write32(dev->bus, dev->slot, PCI_COMMAND_OFFSET, value);
}

int pci_find_device(struct pci_device *dev, uint16_t target_vendor, uint16_t target_device) {
    for (int bus = 0; bus <= 255; bus++) {
        for (int slot = 0; slot < 32; slot++) {
		uint16_t vendor = read16(bus, slot, PCI_VENDOR_ID_OFFSET);
		uint16_t device = read16(bus, slot, PCI_DEVICE_ID_OFFSET);
		if(vendor != target_vendor || device != target_device) {
			continue;
		}
		dev->bus = bus;
		dev->slot = slot;
		dev->vendor = vendor;
		dev->device = device;
		dev->bar0 = read32(bus, slot, PCI_BAR0_OFFSET);
		dev->irq = read8(bus, slot, PCI_INTR_LINE_OFFSET);
		return 1;
        }
    }
    return 0;
}
