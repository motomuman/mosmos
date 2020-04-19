#include "udp.h"
#include "print.h"
#include "netutil.h"
#include "list.h"
#include "lib.h"
#include "memory.h"
#include "ether.h"
#include "ip.h"
#include "netdev.h"
#include "task.h"
#include "workqueue.h"

#define NULL 0

struct udp_socket_pkt {
	struct list_item link;
	struct pktbuf *pkt;
};

struct udp_socket {
	struct listctl pktlist;
	struct TASK *receiver;
	uint16_t port;
	int wait_id;
};

#define UDP_SOCKET_COUNT 10

int next_udp_socket_id = 0;
uint16_t next_udp_socket_port = 5000;
struct udp_socket udp_sockets[UDP_SOCKET_COUNT];

void udp_init()
{
	next_udp_socket_port = 5000;
}

int udp_socket()
{
	if(next_udp_socket_id >= UDP_SOCKET_COUNT) {
		printstr_log("ERROR: exceed max udp socket count\n");
		panic();
	}
	int socket_id = next_udp_socket_id;
	uint16_t port = next_udp_socket_port;
	next_udp_socket_id++;
	next_udp_socket_port++;
	udp_sockets[socket_id].wait_id = 0;
	udp_sockets[socket_id].port = port;
	udp_sockets[socket_id].receiver = current_task();
	list_init(&udp_sockets[socket_id].pktlist);

	return socket_id;
}

void udp_socket_send(int socket_id, uint32_t dip, uint16_t dport, uint8_t *buf, uint32_t size)
{
	printstr_app("udp send\n");
	struct udp_socket socket = udp_sockets[socket_id];

	struct pktbuf * txpkt = (struct pktbuf *)mem_alloc(sizeof(struct pktbuf), "udp_send_pbuf");
	txpkt->pkt_len =  (sizeof(struct ether_hdr)
			+ sizeof(struct ip_hdr)) + sizeof(struct udp_hdr) + size;
	uint8_t *txbuf = (uint8_t *)mem_alloc(sizeof(uint8_t) * txpkt->pkt_len, "udp_send_pbuf_buf");
	txpkt->buf = txbuf;
	txpkt->buf_head = txbuf;

	// reserve for ether header and ip_hdr
	txpkt->buf += sizeof(struct ether_hdr) + sizeof(struct ip_hdr);


	// Dummy ip header for checksum
	txpkt->buf -= sizeof(struct udp_dummy_ip_hdr);
	struct udp_dummy_ip_hdr *dummy_hdr = (struct udp_dummy_ip_hdr *) txpkt->buf;
	txpkt->buf += sizeof(struct udp_dummy_ip_hdr);
	dummy_hdr->sip = hton32(get_netdev()->ip_addr);
	dummy_hdr->dip = hton32(dip);
	dummy_hdr->zero = 0;
	dummy_hdr->proto = IP_HDR_PROTO_UDP;
	dummy_hdr->len = hton16(sizeof(struct udp_hdr) + size);

	struct udp_hdr *udphdr = (struct udp_hdr *) txpkt->buf;
	txpkt->buf += sizeof(struct udp_hdr);
	udphdr->sport = hton16(socket.port);
	udphdr->dport = hton16(dport);
	udphdr->len =  hton16(sizeof(struct udp_hdr) + size);
	udphdr->checksum = 0;


	// set udp data
	memcpy(txpkt->buf, buf, size);
	txpkt->buf -= sizeof(struct udp_hdr);

	//checksum
	txpkt->buf -= sizeof(struct udp_dummy_ip_hdr);
	udphdr->checksum = checksum((uint16_t *)txpkt->buf,
			sizeof(struct udp_dummy_ip_hdr) + sizeof(struct udp_hdr) + size);
	txpkt->buf += sizeof(struct udp_dummy_ip_hdr);

	txpkt->buf -= sizeof(struct ip_hdr);
	ip_tx(txpkt, dip, IP_HDR_PROTO_UDP, 64);

	mem_free(txpkt->buf_head);
	mem_free(txpkt);
}

void udp_socket_recv_timeout(void *_args)
{
	int *args = (int*) _args;
	printstr_app("udp_socket_recv_timeout!!!!\n");
	int socket_id = args[0];
	int wait_id = args[1];
	struct udp_socket *socket = &udp_sockets[socket_id];
	if(socket->wait_id == wait_id){
		task_run(socket->receiver);
	}
	mem_free(args);
}

int udp_socket_recv(int socket_id, uint8_t *buf, uint32_t size)
{
	struct udp_socket *socket = &udp_sockets[socket_id];
	if(list_empty(&socket->pktlist)) {
		printstr_log("udp recv task_sleep\n");
		socket->wait_id++;
		int * args = (int *) mem_alloc(2 * sizeof(int), "udp_timeout_arg");
		args[0] = socket_id;
		args[1] = socket->wait_id;
		wq_push_with_delay(udp_socket_recv_timeout, args, 5000);
		task_sleep();
	}
	if(list_empty(&socket->pktlist)) {
		return -1;
	}

	struct udp_socket_pkt *pkt = (struct udp_socket_pkt *)list_popfront(&socket->pktlist);
	memcpy(buf, pkt->pkt->buf, min_uint32(size, pkt->pkt->pkt_len));
	return 0;
}

void udp_rx(struct pktbuf *pkt)
{
	struct udp_hdr *udphdr = (struct udp_hdr *)pkt->buf;
	pkt->buf += sizeof(struct udp_hdr);
	printstr_app("udp_rx dport:");
	printnum_app(ntoh16(udphdr->dport));
	printstr_app("\n");

	int i;
	for(i = 0; i < next_udp_socket_id; i++){
		printstr_app("waiting port:");
		printnum_app(udp_sockets[i].port);
		printstr_app("\n");
		if(udp_sockets[i].port == ntoh16(udphdr->dport)) {
			// copy pkt
			struct pktbuf * copypkt = (struct pktbuf *)mem_alloc(sizeof(struct pktbuf), "udp_copy_pbuf");
			copypkt->pkt_len = pkt->pkt_len;

			uint8_t *copybuf = (uint8_t *)mem_alloc(sizeof(uint8_t) * copypkt->pkt_len, "udp_copy_pbuf_buf");
			copypkt->buf = copybuf;
			copypkt->buf_head = copybuf;
			memcpy(copypkt->buf, pkt->buf, pkt->pkt_len - sizeof(struct ether_hdr) - sizeof(struct ip_hdr) - sizeof(struct udp_hdr));

			struct udp_socket_pkt * udp_socket_pkt = (struct udp_socket_pkt *) mem_alloc(sizeof(struct udp_socket_pkt), "udp_socket_pkt");
			udp_socket_pkt->pkt = copypkt;
			list_pushback(&udp_sockets[i].pktlist, &udp_socket_pkt->link);
			if(udp_sockets[i].receiver != NULL) {
				task_run(udp_sockets[i].receiver);
			}
		}
	}
}
