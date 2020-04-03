#include "print.h"
#include "pci.h"
#include "r8169.h"

int init_r8169() {
	struct pci_device pcidev;
	pci_find_device(&pcidev, PCI_VENDOR_ID_REALTEK, PCI_DEVICE_ID_REALTEK_810XE);
	pci_enable_bus_master(&pcidev);

	printstr_app("device bus: ");
	printnum_app(pcidev.bus);
	printstr_app(" slot: ");
	printnum_app(pcidev.slot);
	printstr_app(" bar0: ");
	printnum_app(pcidev.bar0);
	printstr_app(" irq: ");
	printnum_app(pcidev.irq);
	printstr_app("\n");

	return 1;
}
