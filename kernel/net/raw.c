#include "list.h"
#include "lib.h"
#include "print.h"
#include "memory.h"
#include "pktbuf.h"
#include "ether.h"
#include "ip.h"
#include "task.h"
#include "workqueue.h"
#include "timer.h"
#include "asm.h"

#define RAW_SOCKET_FREE	0
#define RAW_SOCKET_USED	1

struct raw_rx_data {
	struct list_item link;
	uint8_t *data;
	int data_len;
};

struct raw_socket {
	struct listctl rx_data_list;
	uint64_t wait_id;
	uint8_t proto;
	uint8_t flag;
};

#define RAW_SOCKET_COUNT 10

struct raw_socket raw_sockets[RAW_SOCKET_COUNT];

void raw_socket_init()
{
	int i;
	for(i = 0; i < RAW_SOCKET_COUNT; i++) {
		list_init(&raw_sockets[i].rx_data_list);
		raw_sockets[i].flag = RAW_SOCKET_FREE;
	}
	printstr_log("Initialized RAW socket\n");
}

int raw_socket(uint8_t proto)
{
	int i;
	for(i = 0; i < RAW_SOCKET_COUNT; i++) {
		if(raw_sockets[i].flag == RAW_SOCKET_FREE) {
			raw_sockets[i].wait_id = 0;
			raw_sockets[i].proto = proto;
			raw_sockets[i].flag = RAW_SOCKET_USED;
			return i;
		}
	}
	printstr_log("ERROR: exceed max raw socket count\n");
	panic();
	return -1;
}

void raw_socket_free(int socket_id)
{
	if(socket_id < 0 || socket_id >= RAW_SOCKET_COUNT) {
		printstr_log("ERROR: invalid socket_id\n");
		panic();
	}
	if(raw_sockets[socket_id].flag != RAW_SOCKET_USED) {
		printstr_log("ERROR: Tried to free unused raw socket\n");
		panic();
	}
	raw_sockets[socket_id].flag = RAW_SOCKET_FREE;

	while(!list_empty(&raw_sockets[socket_id].rx_data_list)) {
		struct raw_rx_data *rx_data = (struct raw_rx_data *)list_popfront(&raw_sockets[socket_id].rx_data_list);
		mem_free(rx_data->data);
		mem_free(rx_data);
	}
}

// ttl in args is not good
void raw_socket_send(int socket_id, uint32_t dip, uint8_t *buf, uint32_t size, uint8_t ttl)
{
	if(socket_id < 0 || socket_id >= RAW_SOCKET_COUNT) {
		printstr_log("ERROR: invalid socket_id\n");
		panic();
	}
	struct raw_socket *socket = &raw_sockets[socket_id];

	struct pktbuf * txpkt = (struct pktbuf *)mem_alloc(sizeof(struct pktbuf), "raw_send_pbuf");
	txpkt->pkt_len =  (sizeof(struct ether_hdr) + sizeof(struct ip_hdr)) + size;

	uint8_t *txbuf = (uint8_t *)mem_alloc(sizeof(uint8_t) * txpkt->pkt_len, "raw_send_pbuf_buf");
	txpkt->buf = txbuf;
	txpkt->buf_head = txbuf;

	// reserve for ether header and ip_hdr
	txpkt->buf += sizeof(struct ether_hdr) + sizeof(struct ip_hdr);

	// set ip data
	memcpy(txpkt->buf, buf, size);

	txpkt->buf -= sizeof(struct ip_hdr);
	ip_tx(txpkt, dip, socket->proto, ttl);

	mem_free(txpkt->buf_head);
	mem_free(txpkt);
}

void raw_socket_recv_timeout(void *_args)
{
	int *args = (int*) _args;
	int socket_id = args[0];
	int wait_id = args[1];
	struct raw_socket *socket = &raw_sockets[socket_id];
	if(socket->wait_id == wait_id) {
		task_wakeup(socket);
	}
	mem_free(args);
}

int raw_socket_recv(int socket_id, uint8_t *buf, int size)
{
	if(socket_id < 0 || socket_id >= RAW_SOCKET_COUNT) {
		printstr_log("ERROR: invalid socket_id\n");
		panic();
	}

	struct raw_socket *socket = &raw_sockets[socket_id];
	if(list_empty(&socket->rx_data_list)) {
		// Set timeout and sleep this task
		socket->wait_id = get_tick();
		int * args = (int *) mem_alloc(2 * sizeof(int), "raw_timeout_arg");
		args[0] = socket_id;
		args[1] = socket->wait_id;
		wq_push_with_delay(raw_socket_recv_timeout, args, 5000);
		task_sleep(socket);
	}
	
	// timeout
	if(list_empty(&socket->rx_data_list)) {
		return -1;
	}

	struct raw_rx_data *rx_data = (struct raw_rx_data *)list_popfront(&socket->rx_data_list);
	int data_len = min_int(size, rx_data->data_len);
	memcpy(buf, rx_data->data, data_len);
	return data_len;
}

void raw_recv(struct pktbuf *pkt, uint8_t proto)
{
	int i;
	uint64_t rflags = get_rflags();
	io_cli();
	for(i = 0; i < RAW_SOCKET_COUNT; i++){
		if(raw_sockets[i].flag == RAW_SOCKET_USED && raw_sockets[i].proto == proto) {
			// copy pkt data
			struct raw_rx_data *rx_data = (struct raw_rx_data *) mem_alloc(sizeof(struct raw_rx_data), "rx_data");

			int data_len = pkt->pkt_len - sizeof(struct ether_hdr);
			rx_data->data_len = data_len;

			uint8_t *data = (uint8_t *) mem_alloc(pkt->pkt_len, "rx_data_data");
			rx_data->data = data;
			memcpy(data, pkt->buf, data_len);

			list_pushback(&raw_sockets[i].rx_data_list, &rx_data->link);
			task_wakeup(&raw_sockets[i]);
		}
	}
	set_rflags(rflags);
}
