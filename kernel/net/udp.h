#ifndef _UDP_H_
#define _UDP_H_

#include <stdint.h>
#include "pktbuf.h"

struct udp_hdr {
	uint16_t sport;
	uint16_t dport;
	uint16_t len;
	uint16_t checksum;
} __attribute__ ((packed));

void udp_rx(struct pktbuf *pkt);

#endif
