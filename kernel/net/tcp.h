#ifndef _TCP_H_
#define _TCP_H_

#include <stdint.h>
#include "pktbuf.h"

#define TCP_FLAGS_FIN (1<<0)
#define TCP_FLAGS_SYN (1<<1)
#define TCP_FLAGS_RST (1<<2)
#define TCP_FLAGS_PSH (1<<3)
#define TCP_FLAGS_ACK (1<<4)
#define TCP_FLAGS_URG (1<<5)

struct tcp_hdr {
	uint16_t sport;
	uint16_t dport;
	uint32_t seq_num;
	uint32_t ack_num;
	uint16_t flags;
	uint16_t win_size;
	uint16_t checksum;
	uint16_t urg_ptr;
	//uint32_t opt;
} __attribute__ ((packed));

struct tcp_dummy_ip_hdr {
	uint32_t sip;
	uint32_t dip;
	uint8_t zero;
	uint8_t proto;
	uint16_t len;
} __attribute__ ((packed));


void tcp_socket_init();
int tcp_socket();
int tcp_socket_connect(int socket_id, uint32_t dip, uint16_t dport);
int tcp_socket_send(int socket_id, uint8_t *buf, int size);
void tcp_rx(struct pktbuf *pkt);
int tcp_socket_close(int socket_id);
int tcp_socket_recv(int socket_id, uint8_t *buf, int size);

#endif
