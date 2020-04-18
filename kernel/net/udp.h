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

struct udp_dummy_ip_hdr {
	uint32_t sip;
	uint32_t dip;
	uint8_t zero;
	uint8_t proto;
	uint16_t len;
} __attribute__ ((packed));

void udp_rx(struct pktbuf *pkt);
int udp_socket();
void udp_socket_send(int socket_id, uint32_t dip, uint16_t dport, uint8_t *buf, uint32_t size);

#endif
