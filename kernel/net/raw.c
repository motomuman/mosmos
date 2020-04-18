#include "list.h"
#include "lib.h"
#include "print.h"
#include "memory.h"
#include "pktbuf.h"
#include "ether.h"
#include "ip.h"
#include "task.h"
#include "workqueue.h"

struct raw_socket_pkt {
	struct list_item link;
	struct pktbuf *pkt;
};

struct raw_socket {
	struct listctl pktlist;
	struct TASK *receiver;
	int wait_id;
	uint8_t proto;
};

#define RAW_SOCKET_COUNT 10

int next_raw_socket_id = 0;
struct raw_socket raw_sockets[RAW_SOCKET_COUNT];

int raw_socket(uint8_t proto)
{
	if(next_raw_socket_id >= RAW_SOCKET_COUNT) {
		printstr_log("ERROR: exceed max raw socket count\n");
		panic();
	}
	int socket_id = next_raw_socket_id;
	next_raw_socket_id++;
	raw_sockets[socket_id].wait_id = 0;
	raw_sockets[socket_id].proto = proto;
	raw_sockets[socket_id].receiver = current_task();
	list_init(&raw_sockets[socket_id].pktlist);

	return socket_id;
}

void raw_socket_send(int socket_id, uint32_t dip, uint8_t *buf, uint32_t size, uint8_t ttl)
{
	struct raw_socket *socket = &raw_sockets[socket_id];
	printstr_app("raw send\n");

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
	printstr_app("raw_socket_recv_timeout!!!!\n");
	int socket_id = args[0];
	int wait_id = args[1];
	struct raw_socket *socket = &raw_sockets[socket_id];
	if(socket->wait_id == wait_id){
		task_run(socket->receiver);
	}
	mem_free(args);
}


int raw_socket_recv(int socket_id, uint8_t *buf, uint32_t size)
{
	struct raw_socket *socket = &raw_sockets[socket_id];
	if(list_empty(&socket->pktlist)) {
		printstr_log("recv task_sleep\n");
		socket->wait_id++;
		int * args = (int *) mem_alloc(2 * sizeof(int), "raw_timeout_arg");
		args[0] = socket_id;
		args[1] = socket->wait_id;
		wq_push_with_delay(raw_socket_recv_timeout, args, 5000);
		task_sleep();
	}
	
	if(list_empty(&socket->pktlist)) {
		return -1;
	}

	struct raw_socket_pkt *pkt = (struct raw_socket_pkt *)list_popfront(&socket->pktlist);
	memcpy(buf, pkt->pkt->buf, min_uint32(size, pkt->pkt->pkt_len));
	return 0;
}

void raw_recv(struct pktbuf *pkt, uint8_t proto)
{
	int i;
	for(i = 0; i < next_raw_socket_id; i++){
		if(raw_sockets[i].proto == proto) {
			// copy pkt
			struct pktbuf * copypkt = (struct pktbuf *)mem_alloc(sizeof(struct pktbuf), "raw_copy_pbuf");
			copypkt->pkt_len = pkt->pkt_len; 

			uint8_t *copybuf = (uint8_t *)mem_alloc(sizeof(uint8_t) * copypkt->pkt_len, "raw_copy_pbuf_buf");
			copypkt->buf = copybuf;
			copypkt->buf_head = copybuf;
			memcpy(copypkt->buf, pkt->buf, pkt->pkt_len - sizeof(struct ip_hdr));

			struct raw_socket_pkt * raw_socket_pkt = (struct raw_socket_pkt *) mem_alloc(sizeof(struct raw_socket_pkt), "raw_socket_pkt");
			raw_socket_pkt->pkt = copypkt;
			list_pushback(&raw_sockets[i].pktlist, &raw_socket_pkt->link);
			if(raw_sockets[i].receiver != NULL) {
				task_run(raw_sockets[i].receiver);
			}
		}
	}
}
