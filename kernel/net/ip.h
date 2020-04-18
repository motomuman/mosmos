#ifndef _IP_H_
#define _IP_H_

#include <stdint.h>
#include "pktbuf.h"

#define IP_HDR_PROTO_IPV4_VER 4

#define IP_HDR_PROTO_ICMP 0x01
#define IP_HDR_PROTO_TCP  0x06
#define IP_HDR_PROTO_UDP  0x11

struct ip_hdr {
	uint8_t ver_ihl;
	uint8_t tos;
	uint16_t len;
	uint16_t id;
	uint16_t flafra;
	uint8_t ttl;
	uint8_t proto;
	uint16_t cksum;
	uint32_t sip;
	uint32_t dip;
} __attribute__ ((packed));

void ip_rx(struct pktbuf *pkt);
void ip_tx(struct pktbuf *pkt, uint32_t dip, uint8_t proto, uint8_t ttl);

#endif
