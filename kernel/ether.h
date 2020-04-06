#ifndef _ETHER_H_
#define _ETHER_H_

#include <stdint.h>
#include "pktbuf.h"

#define ETHER_ADDR_LEN 6
#define ETHER_TYPE_IPV4 0x0800
#define ETHER_TYPE_ARP 0x0806

struct ether_hdr {
	uint8_t dmac[ETHER_ADDR_LEN];
	uint8_t smac[ETHER_ADDR_LEN];
	uint16_t type;
};

void ether_rx(struct pktbuf *pkt);
void ether_tx(struct pktbuf *pkt, uint8_t *dst_mac, uint16_t proto);

#endif
