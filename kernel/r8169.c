#include "print.h"
#include "pci.h"
#include "r8169.h"
#include "nasmfunc.h"
#include "memory.h"
#include "int.h"
#include "workqueue.h"

/*
 * References
 * https://wiki.osdev.org/RTL8139
 * https://wiki.osdev.org/RTL8169
 * https://datasheetspdf.com/datasheet/RTL8169.html
 */

// Took constants from linux r8169.c 8139too.c driver
// Removed unused value
enum rtl_registers {
	TxDescStartAddrLow	= 0x20,
	TxDescStartAddrHigh	= 0x24,

	ChipCmd		= 0x37,
#define CmdReset	  0x10
#define CmdRxEnb	  0x08
#define CmdTxEnb	  0x04
#define RxBufEmpty	  0x01

	IntrMask	= 0x3c,
#define PCIErr		  0x8000
#define PCSTimeout	  0x4000
#define RxFIFOOver	  0x40
#define RxUnderrun	  0x20
#define RxOverflow	  0x10
#define TxErr		  0x08
#define TxOK		  0x04
#define RxErr		  0x02
#define RxOK		  0x01

	IntrStatus	= 0x3e,

	TxConfig	= 0x40,
// Interframe Gap Time.
#define TxIFGShift	  24
// 9.6us / 960ns (10 / 100Mbps)
#define TxIFG96		  (3 << TxIFGShift)
#define TxDMAShift	  8
#define TxCfgDMAUnlimited (7 << TxDMAShift)

	RxConfig	= 0x44,
//rx fifo threshold
//111 = No Rx threshold. The RTL8169 begins the transfer or data
//after having received a whole packet in the FIFO
#define RxCfgFIFOShift	  13
#define RxCfgFIFONone	  (7 << RxCfgFIFOShift)
// rx dma
#define RxCfgDMAShift	  8
#define RxCfgDMAUnlimited (7 << RxCfgDMAShift)
// rx mode
#define AcceptErr	  0x20
#define AcceptRunt	  0x10
#define AcceptBroadcast	  0x08
#define AcceptMulticast	  0x04
#define AcceptMyPhys	  0x02
#define AcceptAllPhys	  0x01

	Cfg9346		= 0x50,
#define Cfg9346_Lock	  0x00
#define Cfg9346_Unlock	  0xC0

	Config1		= 0x52,
#define Cfg1_PM_Enable	  0x01

	RxMaxSize	= 0xda,
	RxDescAddrLow	= 0xe4,
	RxDescAddrHigh	= 0xe8,

	EarlyTxThres	= 0xec,	/* 8169. Unit of 32 bytes. */
#define NoEarlyTx	  0x3f	/* Max value : no early transmit. */
};

#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024
#define RX_BUF_SIZE 1536
#define TX_BUF_SIZE 1536

struct Descriptor
{
	uint32_t opts1;		/* command/status uint32_t */
	uint32_t opts2;     	/* currently unused */
	uint32_t low_buf;  	/* low 32-bits of physical buffer address */
	uint32_t high_buf; 	/* high 32-bits of physical buffer address */
};

enum desc_bit {
	// First doubleword. (opts1)
	DescOwn		= (1 << 31), /* Descriptor is owned by NIC */
	RingEnd		= (1 << 30), /* End of descriptor ring */
	FirstFrag	= (1 << 29), /* First segment of a packet */
	LastFrag	= (1 << 28), /* Final segment of a packet */
};

struct r8169_device_t {
	struct pci_device pcidev;

	uint32_t cur_rx;
	uint32_t cur_tx;

	uint8_t rx_buf[RX_RING_SIZE*RX_BUF_SIZE] __attribute__((aligned(8)));
	uint8_t tx_buf_lo[TX_RING_SIZE*TX_BUF_SIZE] __attribute__((aligned(8)));

	struct Descriptor rx_ring[RX_RING_SIZE] __attribute__((aligned(256)));
	struct Descriptor tx_ring[RX_RING_SIZE] __attribute__((aligned(256)));

	uint32_t ioaddr;
};

struct r8169_device_t r8169_device;

void receive_packet() {
	while((io_in8(r8169_device.ioaddr + ChipCmd) & RxBufEmpty) == 0 ) {
		struct Descriptor *rx_desc = r8169_device.rx_ring + r8169_device.cur_rx;

		if( rx_desc->opts1 & DescOwn ) {
			printstr_app("RX processing finished\n");
			break;
		}

		int32_t pkt_len = (rx_desc->opts1 & 0x00001fff); //lower 13bit
		uint8_t *pkt_data = (uint8_t *) rx_desc->low_buf;


		struct work *w = (struct work *)mem_alloc(sizeof(struct work));
		w->type = wt_packet_receive;
		w->u.packet_receive.pkt_len = pkt_len;
		wq_push(w);

		//printstr_app("Packet RX Size: ");
		//printnum_app(pkt_len);
		//printstr_app("\n");
		//uint16_t i;

		//printstr_app("dst mac: ");
		//for (i = 0; i < 6; i++){
		//	printhex_app(*((uint8_t *)(pkt_data + i)));
		//	if(i != 5) {
		//		printstr_app(":");
		//	}
		//}
		//printstr_app("\n");
		//printstr_app("src mac: ");
		//for (i = 6; i < 12; i++){
		//	printhex_app(*((uint8_t *)(pkt_data + i)));
		//	if(i != 11) {
		//		printstr_app(":");
		//	}
		//}
		//printstr_app("\n");
		//printstr_app("type: ");
		//printhex_app(*((uint8_t *)(pkt_data + 12)));
		//printhex_app(*((uint8_t *)(pkt_data + 13)));
		//printstr_app("\n");

		rx_desc->opts1 |= DescOwn;
		r8169_device.cur_rx++;
		if(r8169_device.cur_rx >= RX_RING_SIZE) {
			r8169_device.cur_rx -=RX_RING_SIZE;
		}
	}
}

void r8169_int_handler() {
	uint16_t status = io_in16(r8169_device.ioaddr + IntrStatus);

	printstr_app("r8169_int: "); printhex_app(status); printstr_app("\n");
	if(status & PCIErr) printstr_app("PCIErr\n");
	if(status & PCSTimeout) printstr_app("PCSTimeout\n");
	if(status & RxFIFOOver) printstr_app("RxFIFOOver\n");
	if(status & RxUnderrun) printstr_app("RxUnderrun LinkChange\n");
	if(status & RxOverflow) printstr_app("RxOverflow\n");
	if(status & TxErr) printstr_app("TxErr\n");
	if(status & RxErr) printstr_app("RxErr\n");
	if(status & TxOK) printstr_app("Packet tx\n");
	if(status & RxOK) printstr_app("Packet rx\n");

	if(status & RxOK) {
		receive_packet();
	}

	pic_sendeoi(r8169_device.pcidev.irq);
	io_out16(r8169_device.ioaddr + IntrStatus, status);
}

void send_packet() {
	return;
}

void show_mac() {
	int i;
	printstr_app("mac: ");
	for (i = 0; i < 6; i++) {
		uint8_t mac;
		mac = io_in8(r8169_device.ioaddr + i);
		printhex_app(mac);
		printstr_app(":");
	}
	printstr_app("\n");
}

void setup_rx_descriptors()
{
	int i;
	for(i = 0; i < RX_RING_SIZE; i++)
	{
		r8169_device.rx_ring[i].opts1 = (DescOwn | (RX_BUF_SIZE & 0xFFFF));
		r8169_device.rx_ring[i].opts2 = 0;
		r8169_device.rx_ring[i].low_buf = (uint32_t) (r8169_device.rx_buf) + i * RX_BUF_SIZE;
		r8169_device.rx_ring[i].high_buf = 0;
	}

	r8169_device.rx_ring[RX_RING_SIZE - 1].opts1 = (DescOwn | RingEnd | (RX_BUF_SIZE & 0xFFFF));
}

int init_r8169() {
	r8169_device.cur_rx = 0;
	int ret = pci_find_device(&r8169_device.pcidev, PCI_VENDOR_ID_REALTEK, PCI_DEVICE_ID_REALTEK_810XE);
	if(ret == 0) {
		printstr_log("Failed to find r8169 device\n");
		return 0;
	}

	// 0bit: always 1
	// 1bit: reserved
	// 31-2bit: 4-Byte Aligned base address
	uint32_t ioaddr = r8169_device.pcidev.bar0 & (~0x3);
	r8169_device.ioaddr = ioaddr;

	// PCI Bus Mastering
	pci_enable_bus_master(&r8169_device.pcidev);

	// Turning on the RTL8139
	io_out8(ioaddr + Config1, Cfg1_PM_Enable);

	// Software Reset!
	io_out8(ioaddr + ChipCmd, CmdReset);
	while( (io_in8(ioaddr + ChipCmd) & CmdReset) != 0) {}

	// Init Receive buffer
	setup_rx_descriptors();

	// Unlock config register
	io_out8(ioaddr + Cfg9346, Cfg9346_Unlock);

	// Sets the RE and TE bits high (need to be done before txconfig?)
	io_out8(ioaddr + ChipCmd, 0x0C);

	// RxConfig
	io_out32(ioaddr + RxConfig, RxCfgFIFONone | RxCfgDMAUnlimited |
			AcceptBroadcast | AcceptMulticast | AcceptMyPhys | AcceptAllPhys);
	io_out16(ioaddr + RxMaxSize, RX_BUF_SIZE);
	io_out32(ioaddr + RxDescAddrLow, (uint32_t)&r8169_device.rx_ring[0]);
	io_out32(ioaddr + RxDescAddrHigh, 0);

	// TxConfig
	io_out32(ioaddr + TxConfig, TxIFG96 | TxCfgDMAUnlimited);
	io_out8(ioaddr + EarlyTxThres, NoEarlyTx);
	io_out32(ioaddr + TxDescStartAddrLow, (uint32_t)&r8169_device.tx_ring[0]);
	io_out32(ioaddr + TxDescStartAddrHigh, 0);

	// Enable interruption
	io_out16(ioaddr + IntrMask, PCIErr | PCSTimeout | RxOverflow |
			RxFIFOOver | RxUnderrun | TxErr | TxOK | RxErr | RxOK);

	// Lock config register
	io_out8(ioaddr + Cfg9346, Cfg9346_Lock);

	// register interrupt handler
	register_interrupt(r8169_device.pcidev.irq, (uint32_t) asm_int_r8169);

	return 1;
}
