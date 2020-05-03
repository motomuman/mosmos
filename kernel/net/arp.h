#ifndef _ARP_H_
#define _ARP_H_

#include "ether.h"
#include "types.h"

#define ARP_HARD_TYPE_ETHER 0x0001
#define ARP_PROTO_TCPIP 0x0800

#define ARP_HLEN_ETHER 6
#define ARP_PLEN_ETHER 4

#define	ARP_OP_REQUEST 1
#define	ARP_OP_RESPONSE 2

struct arp_hdr {
	uint16_t hard_type;
	uint16_t proto;
	uint8_t hlen;
	uint8_t plen;
	uint16_t opcode;
};

struct arp_etherip {
	uint8_t smac[ETHER_ADDR_LEN];
	uint32_t sip;
	uint8_t dmac[ETHER_ADDR_LEN];
	uint32_t dip;
} __attribute__ ((packed));

void arp_rx(struct pktbuf *pkt);
void init_arptable();
uint8_t* find_mac_addr(uint32_t ip);
void arp_tx(uint32_t dip);

#endif
