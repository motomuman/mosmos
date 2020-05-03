#include "types.h"
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
#include "timer.h"
#include "asm.h"

#define UDP_SOCKET_FREE	0
#define UDP_SOCKET_USED	1

struct udp_rx_data {
	struct list_item link;
	uint8_t *data;
	int data_len;
};

struct udp_socket {
	struct listctl rx_data_list;
	uint64_t wait_id;
	uint16_t port;
	uint8_t flag;
};

#define UDP_SOCKET_COUNT 10
#define UDP_PORT_START 5000

struct udp_socket udp_sockets[UDP_SOCKET_COUNT];

void udp_socket_init()
{
	int i;
	for(i = 0; i < UDP_SOCKET_COUNT; i++) {
		list_init(&udp_sockets[i].rx_data_list);
		udp_sockets[i].flag = UDP_SOCKET_FREE;
	}
	printstr_log("Initialized UDP socket\n");
}

int udp_socket()
{
	int i;
	for(i = 0; i < UDP_SOCKET_COUNT; i++) {
		if(udp_sockets[i].flag == UDP_SOCKET_FREE) {
			udp_sockets[i].wait_id = 0;
			udp_sockets[i].flag = UDP_SOCKET_USED;
			udp_sockets[i].port = UDP_PORT_START + i;
			return i;
		}
	}
	printstr_log("ERROR: exceed max udp socket count\n");
	panic();
	return -1;


}

void udp_socket_free(int socket_id)
{
	if(socket_id < 0 || socket_id >= UDP_SOCKET_COUNT) {
		printstr_log("ERROR: invalid udp socket_id\n");
		panic();
	}
	if(udp_sockets[socket_id].flag != UDP_SOCKET_USED) {
		printstr_log("ERROR: Tried to free unused udp socket\n");
		panic();
	}
	udp_sockets[socket_id].flag = UDP_SOCKET_FREE;

	while(!list_empty(&udp_sockets[socket_id].rx_data_list)) {
		struct udp_rx_data *rx_data = (struct udp_rx_data *)list_popfront(&udp_sockets[socket_id].rx_data_list);
		mem_free(rx_data->data);
		mem_free(rx_data);
	}
}

void udp_socket_send(int socket_id, uint32_t dip, uint16_t dport, uint8_t *buf, uint32_t size)
{
	if(socket_id < 0 || socket_id >= UDP_SOCKET_COUNT) {
		printstr_log("ERROR: invalid udp socket_id\n");
		panic();
	}
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

	//send pkt
	txpkt->buf -= sizeof(struct ip_hdr);
	ip_tx(txpkt, dip, IP_HDR_PROTO_UDP, 64);

	mem_free(txpkt->buf_head);
	mem_free(txpkt);
}

void udp_socket_recv_timeout(void *_args)
{
	int *args = (int*) _args;
	int socket_id = args[0];
	int wait_id = args[1];
	struct udp_socket *socket = &udp_sockets[socket_id];
	if(socket->wait_id == wait_id) {
		task_wakeup(socket);
	}
	mem_free(args);
}

int udp_socket_recv(int socket_id, uint8_t *buf, int size)
{
	if(socket_id < 0 || socket_id >= UDP_SOCKET_COUNT) {
		printstr_log("ERROR: invalid udp socket_id\n");
		panic();
	}

	struct udp_socket *socket = &udp_sockets[socket_id];
	if(list_empty(&socket->rx_data_list)) {
		// set timeout and sleep this task
		socket->wait_id = get_tick();
		int * args = (int *) mem_alloc(2 * sizeof(int), "udp_timeout_arg");
		args[0] = socket_id;
		args[1] = socket->wait_id;
		wq_push_with_delay(udp_socket_recv_timeout, args, 5000);
		task_sleep(socket);
	}

	//timeout
	if(list_empty(&socket->rx_data_list)) {
		return -1;
	}

	struct udp_rx_data *rx_data = (struct udp_rx_data *)list_popfront(&socket->rx_data_list);
	int data_len = min_int(size, rx_data->data_len);
	memcpy(buf, rx_data->data, data_len);
	return data_len;
}

void udp_rx(struct pktbuf *pkt)
{
	struct udp_hdr *udphdr = (struct udp_hdr *)pkt->buf;
	pkt->buf += sizeof(struct udp_hdr);

	int i;
	uint64_t rflags = get_rflags();
	io_cli();

	for(i = 0; i < UDP_SOCKET_COUNT; i++){
		if(udp_sockets[i].flag == UDP_SOCKET_USED && udp_sockets[i].port == ntoh16(udphdr->dport)) {
			// copy pkt data
			struct udp_rx_data *rx_data = (struct udp_rx_data *) mem_alloc(sizeof(struct udp_rx_data), "rx_data");

			int data_len = pkt->pkt_len - sizeof(struct ether_hdr) - sizeof(struct ip_hdr) - sizeof(struct udp_hdr);
			rx_data->data_len = data_len;

			uint8_t *data = (uint8_t *) mem_alloc(pkt->pkt_len, "rx_data_data");
			rx_data->data = data;
			memcpy(data, pkt->buf, data_len);

			list_pushback(&udp_sockets[i].rx_data_list, &rx_data->link);
			task_wakeup(&udp_sockets[i]);
		}
	}
	set_rflags(rflags);
}
