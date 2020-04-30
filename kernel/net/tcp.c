#include "tcp.h"
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

/*
 * Currently
 * 	Option is not implemented
 * 	RxBuffering/Reordering is not implemented
 */

#define NULL 0
#define true 1
#define false 0

#define TCP_SOCKET_FREE	0
#define TCP_SOCKET_USED	1

typedef enum TCP_STATE {
	CLOSED,
	LISTEN,
	SYN_RCVD,
	SYN_SENT,
	ESTABLISHED,
	FIN_WAIT_1,
	CLOSING,
	CLOSE_WAIT,
} TCP_STATE;

typedef enum TCP_EVENT {
	EVENT_CONNECT,
	EVENT_TIMEOUT_SYN_ACK,
	EVENT_CLOSE,
	EVENT_RECV_PKT,
} TCP_EVENT;

struct tcp_rx_data {
	struct list_item link;
	uint8_t *data;
	int data_len;
};

#define RETRANSMIT_DUP_ACK_NUM 3
// This should be dynamic
#define RETRANSMIT_ACK_TIMEOUT_MSEC 1000

// To store transmitted packet.
// When socket received ack for this pkt,
// Remove this info
struct tcp_tx_data {
	struct list_item link;
	int socket_id;
	uint32_t send_seq_num; //seq number of this pkt
	uint32_t wait_ack_num; //ack number for this pkt (seq num + data size)
	uint32_t dup_ack_num;
	struct pktbuf * txpkt;
};

struct tcp_socket {
	struct listctl rx_data_list;
	struct listctl tx_data_list;
	uint64_t wait_id;
	uint16_t sport;
	uint32_t sip;
	uint16_t dport;
	uint32_t dip;
	uint8_t flag;

	uint32_t seq_num; //next tx seq num
	uint32_t ack_num; //ack num (next waiting rx seq num)
	uint16_t win_size;
	TCP_STATE state;
};

#define TCP_SOCKET_COUNT 10
#define TCP_PORT_START 6000

struct tcp_socket tcp_sockets[TCP_SOCKET_COUNT];

char *get_tcp_state_str(TCP_STATE state) {
	switch(state) {
		case CLOSED:
			return "CLOSED";
		case LISTEN:
			return "LISTEN";
		case SYN_RCVD:
			return "SYN_RCVD";
		case SYN_SENT:
			return "SYN_SENT";
		case ESTABLISHED:
			return "ESTABLISHED";
		case FIN_WAIT_1:
			return "FIN_WAIT_1";
		case CLOSING:
			return "CLOSING";
		case CLOSE_WAIT:
			return "CLOSE_WAIT";
	}
	return "UNKNOWN_STATE";
}

char *get_tcp_event_str(TCP_EVENT event) {
	switch(event) {
		case EVENT_CONNECT:
			return "EVENT_CONNECT";
		case EVENT_TIMEOUT_SYN_ACK:
			return "EVENT_TIMEOUT_SYN_ACK";
		case EVENT_RECV_PKT:
			return "EVENT_RECV_PKT";
		case EVENT_CLOSE:
			return "EVENT_CLOSE";
	}
	return "UNKNOWN_EVENT";
}

void tcp_socket_init()
{
	int i;
	for(i = 0; i < TCP_SOCKET_COUNT; i++) {
		list_init(&tcp_sockets[i].rx_data_list);
		list_init(&tcp_sockets[i].tx_data_list);
		tcp_sockets[i].flag = TCP_SOCKET_FREE;
	}
}

int tcp_socket()
{
	int i;
	for(i = 0; i < TCP_SOCKET_COUNT; i++) {
		if(tcp_sockets[i].flag == TCP_SOCKET_FREE) {
			tcp_sockets[i].wait_id = 0;
			tcp_sockets[i].flag = TCP_SOCKET_USED;
			tcp_sockets[i].sport = TCP_PORT_START + (get_tick() * get_tick() + get_tick()) % 10000;
			tcp_sockets[i].state = CLOSED;
			tcp_sockets[i].seq_num = 0;
			tcp_sockets[i].ack_num = 0;
			tcp_sockets[i].win_size = 65535;
			return i;
		}
	}
	printstr_log("ERROR: exceed max tcp socket count\n");
	panic();
	return -1;
}

void tcp_send(int socket_id, uint32_t dip, uint16_t dport, uint8_t *buf, uint32_t size, uint8_t flags, int need_ack);

void tcp_connect_timeout(void *_args)
{
	int socket_id = *(int*) _args;
	struct tcp_socket *socket = &tcp_sockets[socket_id];

	switch(socket->state) {
		case SYN_SENT:
			socket->state = CLOSED;
			printstr_log("tcp_state: SYN_SENT -> CLOSED\n");
			task_wakeup(socket);
			break;
		case CLOSED:
		case LISTEN:
		case SYN_RCVD:
		case CLOSE_WAIT:
		case ESTABLISHED:
		case FIN_WAIT_1:
		case CLOSING:
			break;
	}
	mem_free(_args);
}

int tcp_socket_connect(int socket_id, uint32_t dip, uint16_t dport)
{
	struct tcp_socket *socket = &tcp_sockets[socket_id];
	socket->sport = TCP_PORT_START + (get_tick() * get_tick() + get_tick()) % 10000;
	socket->dip = dip;
	socket->dport = dport;

	switch(socket->state) {
		case CLOSED:
			//send syn
			tcp_send(socket_id, socket->dip, socket->dport, NULL, 0, TCP_FLAGS_SYN, true);
			socket->seq_num++;
			socket->state = SYN_SENT;

			// set timeout
			int * args = (int *) mem_alloc(1 * sizeof(int), "tcp_timeout_arg");
			*args = socket_id;
			wq_push_with_delay(tcp_connect_timeout, args, 5000);

			printstr_log("tcp_state: CLOSED -> SYN_SENT\n");

			// wait syn ack
			task_sleep(socket);
			return socket->state == ESTABLISHED;
		case SYN_SENT:
		case LISTEN:
		case SYN_RCVD:
		case CLOSE_WAIT:
		case ESTABLISHED:
		case FIN_WAIT_1:
		case CLOSING:
			printstr_log("tcp_socket_connect, invalid state: ");
			printstr_log(get_tcp_state_str(socket->state));
			printstr_log("\n");
			panic();
			return -1;
	}
	return -1;
}

int tcp_socket_bind(int socket_id, uint32_t sip, uint16_t sport)
{
	struct tcp_socket *socket = &tcp_sockets[socket_id];
	switch(socket->state) {
		case CLOSED:
			socket->sport = sport;
			socket->sip = sip;
			return 0;
		case SYN_SENT:
		case ESTABLISHED:
		case FIN_WAIT_1:
		case CLOSING:
		case LISTEN:
		case SYN_RCVD:
		case CLOSE_WAIT:
			printstr_log("tcp_socket_bind, invalid state: ");
			printstr_log(get_tcp_state_str(socket->state));
			printstr_log("\n");
			panic();
			return -1;
	}
	return -1;
}

int tcp_socket_listen(int socket_id)
{
	struct tcp_socket *socket = &tcp_sockets[socket_id];

	switch(socket->state) {
		case CLOSED:
			socket->state = LISTEN;
			printstr_log("tcp_state: CLOSED -> LISTEN\n");
			return 0;
		case SYN_SENT:
		case CLOSE_WAIT:
		case LISTEN:
		case SYN_RCVD:
		case ESTABLISHED:
		case FIN_WAIT_1:
		case CLOSING:
			printstr_log("tcp_socket_listen, invalid state: ");
			printstr_log(get_tcp_state_str(socket->state));
			printstr_log("\n");
			panic();
			return -1;
	}
	return -1;
}

int tcp_socket_accept(int socket_id)
{
	struct tcp_socket *socket = &tcp_sockets[socket_id];

	switch(socket->state) {
		case LISTEN:
			task_sleep(socket);
			return socket->state == LISTEN ? -1 : 0;
		case CLOSED:
		case SYN_SENT:
		case SYN_RCVD:
		case CLOSE_WAIT:
		case ESTABLISHED:
		case FIN_WAIT_1:
		case CLOSING:
			printstr_log("tcp_socket_accept, invalid state: ");
			printstr_log(get_tcp_state_str(socket->state));
			printstr_log("\n");
			panic();
			return -1;
	}
	return -1;
}

int tcp_socket_close(int socket_id)
{
	struct tcp_socket *socket = &tcp_sockets[socket_id];
	switch(socket->state) {
		case ESTABLISHED:
			tcp_send(socket_id, socket->dip, socket->dport, NULL, 0, TCP_FLAGS_ACK | TCP_FLAGS_FIN, true);
			socket->seq_num++;
			socket->state = FIN_WAIT_1;
			printstr_log("tcp_state: ESTABLISHED -> FIN_WAIT_1\n");
			task_sleep(socket);
			return socket->state == CLOSED ? -1 : 0;
		case CLOSED:
			return 0;
		case CLOSING:
			task_sleep(socket);
			return socket->state == CLOSED ? -1 : 0;
		case SYN_SENT:
		case FIN_WAIT_1:
		case LISTEN:
		case SYN_RCVD:
		case CLOSE_WAIT:
			printstr_log("tcp_socket_close, invalid state: ");
			printstr_log(get_tcp_state_str(socket->state));
			printstr_log("\n");
			panic();
			return -1;
	}
	return -1;
}

void tcp_ack_timeout(void *_args)
{
	int *args = (int*) _args;
	int socket_id = args[0];
	int seq_num = args[1];
	struct tcp_socket *socket = &tcp_sockets[socket_id];
	struct tcp_tx_data *tx_data;
	int already_acked = 1;

	if(socket->state == CLOSED) {
		mem_free(args);
		return;
	}

	//printstr_app("tcp_ack_timeout for ");
	//printnum_app(seq_num);
	//printstr_app("\n");
	for(tx_data = (struct tcp_tx_data *)list_head(&socket->tx_data_list);
			tx_data != NULL; tx_data = (struct tcp_tx_data *)list_next(&tx_data->link)){
		if(tx_data->send_seq_num == seq_num) {
			//printstr_app("tcp_ack_timeout, retransmit\n");
			tx_data->txpkt->buf = tx_data->txpkt->buf_head + sizeof(struct ether_hdr);
			ip_tx(tx_data->txpkt, socket->dip, IP_HDR_PROTO_TCP, 64);
			already_acked = 0;
		}
	}
	if(already_acked) {
		mem_free(args);
	} else {
		wq_push_with_delay(tcp_ack_timeout, _args, RETRANSMIT_ACK_TIMEOUT_MSEC);
	}
}


void tcp_send(int socket_id, uint32_t dip, uint16_t dport, uint8_t *buf, uint32_t size, uint8_t flags, int need_ack)
{
	if(socket_id < 0 || socket_id >= TCP_SOCKET_COUNT) {
		printstr_log("ERROR: invalid tcp socket_id\n");
		panic();
	}
	struct tcp_socket *socket = &tcp_sockets[socket_id];

	int tcp_hdr_len = sizeof(struct tcp_hdr);
	if(socket->state == CLOSED) {
		tcp_hdr_len += 4;
	}

	struct pktbuf * txpkt = (struct pktbuf *)mem_alloc(sizeof(struct pktbuf), "tcp_send_pbuf");
	txpkt->pkt_len =  (sizeof(struct ether_hdr)
			+ sizeof(struct ip_hdr)) + tcp_hdr_len + size;
	uint8_t *txbuf = (uint8_t *)mem_alloc(sizeof(uint8_t) * txpkt->pkt_len, "tcp_send_pbuf_buf");
	txpkt->buf = txbuf;
	txpkt->buf_head = txbuf;

	// reserve for ether header and ip_hdr
	txpkt->buf += sizeof(struct ether_hdr) + sizeof(struct ip_hdr);

	// Dummy ip header for checksum
	txpkt->buf -= sizeof(struct tcp_dummy_ip_hdr);
	struct tcp_dummy_ip_hdr *dummy_hdr = (struct tcp_dummy_ip_hdr *) txpkt->buf;
	txpkt->buf += sizeof(struct tcp_dummy_ip_hdr);
	dummy_hdr->sip = hton32(get_netdev()->ip_addr);
	dummy_hdr->dip = hton32(dip);
	dummy_hdr->zero = 0;
	dummy_hdr->proto = IP_HDR_PROTO_TCP;
	dummy_hdr->len = hton16(tcp_hdr_len + size);

	struct tcp_hdr *tcphdr = (struct tcp_hdr *) txpkt->buf;
	txpkt->buf += sizeof(struct tcp_hdr);
	tcphdr->sport = hton16(socket->sport);
	tcphdr->dport = hton16(dport);
	tcphdr->seq_num = hton32(socket->seq_num);
	tcphdr->ack_num = hton32(socket->ack_num);
	tcphdr->flags = hton16(((tcp_hdr_len/4)<<12) + flags);
	tcphdr->win_size = hton16(socket->win_size);
	tcphdr->checksum = 0;
	tcphdr->urg_ptr = 0;


	// MSS opt, need to support opt
	// in more flexible way
	if(socket->state == CLOSED) {
		txpkt->buf[0] = 0x02;
		txpkt->buf[1] = 0x04;
		txpkt->buf[2] = 0x05;
		txpkt->buf[3] = 0xb4;
		txpkt->buf += 4;
	}

	// set udp data
	memcpy(txpkt->buf, buf, size);
	txpkt->buf -= tcp_hdr_len;

	//checksum
	txpkt->buf -= sizeof(struct tcp_dummy_ip_hdr);
	tcphdr->checksum = checksum((uint16_t *)txpkt->buf,
			sizeof(struct tcp_dummy_ip_hdr) + tcp_hdr_len + size);
	txpkt->buf += sizeof(struct tcp_dummy_ip_hdr);

	//send pkt
	txpkt->buf -= sizeof(struct ip_hdr);
	ip_tx(txpkt, dip, IP_HDR_PROTO_TCP, 64);

	if(need_ack) {
		//store tx pkt to wait ack
		struct tcp_tx_data *tx_data = (struct tcp_tx_data *) mem_alloc(sizeof(struct tcp_tx_data), "tx_data");
		tx_data->socket_id = socket_id;

		tx_data->send_seq_num = socket->seq_num;
		socket->seq_num += size;
		tx_data->wait_ack_num = socket->seq_num;
		tx_data->dup_ack_num = 0;

		tx_data->txpkt = txpkt;
		//printstr_app("push pkt s: ");
		//printnum_app(tx_data->send_seq_num);
		//printstr_app(" a: ");
		//printnum_app(tx_data->wait_ack_num);
		//printstr_app("\n");
		list_pushback(&socket->tx_data_list, &tx_data->link);

		//set timeout for ack
		int * args = (int *) mem_alloc(2 * sizeof(int), "tcp_ack_timeout_arg");
		args[0] = socket_id;
		args[1] = tx_data->send_seq_num;
		wq_push_with_delay(tcp_ack_timeout, args, RETRANSMIT_ACK_TIMEOUT_MSEC);
	} else {
		mem_free(txpkt->buf_head);
		mem_free(txpkt);
	}
}

int tcp_socket_send(int socket_id, uint8_t *buf, int size)
{
	struct tcp_socket *socket = &tcp_sockets[socket_id];
	switch(socket->state) {
		case ESTABLISHED:
			tcp_send(socket_id, socket->dip, socket->dport, buf, size, TCP_FLAGS_ACK, true);
			return 0;
		case CLOSED:
		case SYN_SENT:
		case FIN_WAIT_1:
		case LISTEN:
		case SYN_RCVD:
		case CLOSE_WAIT:
		case CLOSING:
			printstr_log("tcp_socket_send, invalid state: ");
			printstr_log(get_tcp_state_str(socket->state));
			printstr_log("\n");
			panic();
			return -1;
	}
	return -1;
}

void tcp_socket_recv_timeout(void *_args)
{
	int *args = (int*) _args;
	int socket_id = args[0];
	int wait_id = args[1];
	struct tcp_socket *socket = &tcp_sockets[socket_id];
	if(socket->wait_id == wait_id) {
		task_wakeup(socket);
	}
	mem_free(args);
}

int tcp_socket_recv(int socket_id, uint8_t *buf, int size, int timeout_msec)
{
	if(socket_id < 0 || socket_id >= TCP_SOCKET_COUNT) {
		printstr_log("ERROR: invalid tcp socket_id\n");
		panic();
	}

	struct tcp_socket *socket = &tcp_sockets[socket_id];
	if(list_empty(&socket->rx_data_list)) {
		if(socket->state == ESTABLISHED) {
			if(timeout_msec <= 0) {
				return 0;
			}
			// set timeout and sleep this task
			socket->wait_id = get_tick();
			int * args = (int *) mem_alloc(2 * sizeof(int), "tcp_timeout_arg");
			args[0] = socket_id;
			args[1] = socket->wait_id;
			wq_push_with_delay(tcp_socket_recv_timeout, args, timeout_msec);
			task_sleep(socket);
		} else {
			return -1;
		}
	}

	//timeout
	if(list_empty(&socket->rx_data_list)) {
		return 0;
	}

	struct tcp_rx_data *rx_data = (struct tcp_rx_data *)list_popfront(&socket->rx_data_list);
	int data_len = min_int(size, rx_data->data_len);
	memcpy(buf, rx_data->data, data_len);
	return data_len;
}

void show_tx_buf(struct tcp_socket *socket){
	struct tcp_tx_data *tx_data;

	//detect dup ack. check retransmit necessity
	for(tx_data = (struct tcp_tx_data *)list_head(&socket->tx_data_list);
			tx_data != NULL; tx_data = (struct tcp_tx_data *)list_next(&tx_data->link)){
		printstr_app("s: ");
		printnum_app(tx_data->send_seq_num);
		printstr_app("a: ");
		printnum_app(tx_data->wait_ack_num);
		printstr_app(" -> ");
	}
	printstr_app("\n");
}

void handle_ack(struct tcp_socket *socket, struct tcp_hdr *tcphdr){
	//show_tx_buf(socket);

	//printstr_app("handle ack ");
	//printnum_app(ntoh32(tcphdr->ack_num));
	//printstr_app(" ");

	//check buffered tx pkt and remove acked pkt
	struct tcp_tx_data *prev_tx_data;
	struct tcp_tx_data *tx_data;
	int find_ack = 0;

	prev_tx_data = (struct tcp_tx_data *)list_head(&socket->tx_data_list);
	if(prev_tx_data == NULL) {
		return;
	}
	tx_data = (struct tcp_tx_data *)list_next(&prev_tx_data->link);

	//check equal and after 2nd item
	for(;tx_data != NULL; tx_data = (struct tcp_tx_data *)list_next(&tx_data->link)){
		if(tx_data->wait_ack_num <= ntoh32(tcphdr->ack_num)) {
			printstr_app("find acked pkt: ");
			printnum_app(tx_data->wait_ack_num);
			printstr_app("\n");
			list_remove(&socket->tx_data_list, &prev_tx_data->link);
			mem_free(tx_data->txpkt->buf_head);
			mem_free(tx_data->txpkt);
			mem_free(tx_data);
			find_ack = 1;
		}
		prev_tx_data = tx_data;
	}

	// check head item
	tx_data = (struct tcp_tx_data *)list_head(&socket->tx_data_list);
	if(tx_data->wait_ack_num <= ntoh32(tcphdr->ack_num)) {
		printstr_app("find acked pkt: ");
		printnum_app(tx_data->wait_ack_num);
		printstr_app("\n");
		list_popfront(&socket->tx_data_list);
		mem_free(tx_data->txpkt->buf_head);
		mem_free(tx_data->txpkt);
		mem_free(tx_data);
		find_ack = 1;
	}

	if(find_ack) {
		return;
	}

	//detect dup ack. check retransmit necessity
	printstr_app("DETECT DUPPED ACK ack: ");
	printnum_app(ntoh32(tcphdr->ack_num));
	printstr_app("\n");
	for(tx_data = (struct tcp_tx_data *)list_head(&socket->tx_data_list);
			tx_data != NULL; tx_data = (struct tcp_tx_data *)list_next(&tx_data->link)){
		if(tx_data->send_seq_num == ntoh32(tcphdr->ack_num)) {
			tx_data->dup_ack_num++;
			printstr_app("dup_count: ");
			printnum_app(tx_data->dup_ack_num);
			printstr_app("\n");
			if(tx_data->dup_ack_num >= RETRANSMIT_DUP_ACK_NUM) {
				printstr_app("retransmit: ");
				printnum_app(tx_data->txpkt->pkt_len);
				printstr_app("\n");

				tx_data->txpkt->buf = tx_data->txpkt->buf_head + sizeof(struct ether_hdr);
				ip_tx(tx_data->txpkt, socket->dip, IP_HDR_PROTO_TCP, 64);
			}
		}
	}
}

void tcp_recv_pkt(int socket_id, struct pktbuf *pkt)
{
	struct tcp_socket *socket = &tcp_sockets[socket_id];
	struct tcp_hdr *tcphdr = (struct tcp_hdr *)pkt->buf;
	pkt->buf -= sizeof(struct ip_hdr);
	struct ip_hdr *iphdr = (struct ip_hdr *)pkt->buf;
	pkt->buf += sizeof(struct ip_hdr);

	uint32_t ip_len = hton16(iphdr->len);
	uint32_t tcphdr_len = ((hton16(tcphdr->flags) >> 12) & 0xf) * 4;
	uint32_t data_len = ip_len - sizeof(struct ip_hdr) - tcphdr_len;

	switch(socket->state) {
		case CLOSED:
			break;
		case LISTEN:
			if((ntoh16(tcphdr->flags) & TCP_FLAGS_SYN)) {
				socket->ack_num = ntoh32(tcphdr->seq_num) + 1;
				socket->dip = ntoh32(iphdr->sip);
				socket->dport = ntoh16(tcphdr->sport);
				tcp_send(socket_id, socket->dip, socket->dport, NULL, 0, TCP_FLAGS_SYN | TCP_FLAGS_ACK, true);
				socket->seq_num++;
				socket->state = SYN_RCVD;
				printstr_log("tcp_state: LISTEN -> SYN_RCVD\n");
			}
			break;
		case SYN_RCVD:
			pkt->buf += tcphdr_len;
			if((ntoh16(tcphdr->flags) & TCP_FLAGS_ACK)) {
				handle_ack(socket, tcphdr);

				// ack pkt could contain data
				if(data_len > 0) {
					printstr_app("ack in 3 hand shake contains data!!\n");
					// copy pkt data
					struct tcp_rx_data *rx_data = (struct tcp_rx_data *) mem_alloc(sizeof(struct tcp_rx_data), "rx_data");
					rx_data->data_len = data_len;
					uint8_t *data = (uint8_t *) mem_alloc(data_len, "rx_data_data");
					rx_data->data = data;
					memcpy(data, pkt->buf, data_len);
					list_pushback(&socket->rx_data_list, &rx_data->link);
					socket->ack_num = ntoh32(tcphdr->seq_num) + data_len;
				}

				socket->state = ESTABLISHED;
				printstr_log("tcp_state: SYN_RCVD -> ESTABLISHED\n");
				task_wakeup(socket);
			}
			break;
		case SYN_SENT:
			if((ntoh16(tcphdr->flags) & TCP_FLAGS_SYN) && (ntoh16(tcphdr->flags) & TCP_FLAGS_ACK)) {
				handle_ack(socket, tcphdr);
				socket->ack_num = ntoh32(tcphdr->seq_num) + 1;
				tcp_send(socket_id, socket->dip, socket->dport, NULL, 0, TCP_FLAGS_ACK, false);
				socket->state = ESTABLISHED;
				printstr_log("tcp_state: SYN_SENT -> ESTABLISHED\n");
				task_wakeup(socket);
			}
			break;
		case ESTABLISHED:
			pkt->buf += tcphdr_len;
			if(data_len > 0) {
				// copy pkt data
				struct tcp_rx_data *rx_data = (struct tcp_rx_data *) mem_alloc(sizeof(struct tcp_rx_data), "rx_data");
				rx_data->data_len = data_len;
				uint8_t *data = (uint8_t *) mem_alloc(data_len, "rx_data_data");
				rx_data->data = data;
				memcpy(data, pkt->buf, data_len);

				if(socket->ack_num != ntoh32(tcphdr->seq_num)) {
					//PKT LOSS
					//Currently, just skip and send dup ack
					//Maybe need to buffer and use after reordering?
					//Maybe it's better to use ring buffer for rx data
					//so that we can just write this data to ring buffer
					printstr_app("pkt loss\n");
					tcp_send(socket_id, socket->dip, socket->dport, NULL, 0, TCP_FLAGS_ACK, false);
					break;
				}

				list_pushback(&socket->rx_data_list, &rx_data->link);
				socket->ack_num = ntoh32(tcphdr->seq_num) + data_len;

				task_wakeup(socket);
			}


			if(ntoh16(tcphdr->flags) & TCP_FLAGS_ACK) {
				handle_ack(socket, tcphdr);
			}

			if((ntoh16(tcphdr->flags) & TCP_FLAGS_FIN) && (ntoh16(tcphdr->flags) & TCP_FLAGS_ACK)) {
				socket->ack_num++;
				tcp_send(socket_id, socket->dip, socket->dport, NULL, 0, TCP_FLAGS_ACK | TCP_FLAGS_FIN, true);
				socket->state = CLOSING;
				printstr_log("tcp_state: ESTABLISHED -> CLOSING\n");
			} else if (data_len > 0) {
				tcp_send(socket_id, socket->dip, socket->dport, NULL, 0, TCP_FLAGS_ACK, false);
			}
			break;
		case FIN_WAIT_1:
			if(ntoh16(tcphdr->flags) & TCP_FLAGS_ACK) {
				handle_ack(socket, tcphdr);
			}

			if((ntoh16(tcphdr->flags) & TCP_FLAGS_FIN) && (ntoh16(tcphdr->flags) & TCP_FLAGS_ACK)) {
				socket->ack_num++;
				tcp_send(socket_id, socket->dip, socket->dport, NULL, 0, TCP_FLAGS_ACK, false);
				socket->state = CLOSED;
				printstr_log("tcp_state: FIN_WAIT_1 -> CLOSED\n");
				task_wakeup(socket);
			}
			break;
		case CLOSING:
			if((ntoh16(tcphdr->flags) & TCP_FLAGS_ACK)) {
				socket->state = CLOSED;
				printstr_log("tcp_state: CLOSING -> CLOSED\n");
				task_wakeup(socket);
			}
			break;
		default:
			break;
	}
	return;
}

void tcp_rx(struct pktbuf *pkt)
{
	struct tcp_hdr *tcphdr = (struct tcp_hdr *)pkt->buf;

	int i;
	uint64_t rflags = get_rflags();
	io_cli();

	for(i = 0; i < TCP_SOCKET_COUNT; i++){
		if(tcp_sockets[i].flag == TCP_SOCKET_USED && tcp_sockets[i].sport == ntoh16(tcphdr->dport)) {
			//if(ntoh16(tcphdr->flags) & TCP_FLAGS_FIN) printstr_log("TCP_FLAGS_FIN\n");
			//if(ntoh16(tcphdr->flags) & TCP_FLAGS_SYN) printstr_log("TCP_FLAGS_SYN\n");
			//if(ntoh16(tcphdr->flags) & TCP_FLAGS_RST) printstr_log("TCP_FLAGS_RST\n");
			//if(ntoh16(tcphdr->flags) & TCP_FLAGS_PSH) printstr_log("TCP_FLAGS_PSH\n");
			//if(ntoh16(tcphdr->flags) & TCP_FLAGS_ACK) printstr_log("TCP_FLAGS_ACK\n");
			//if(ntoh16(tcphdr->flags) & TCP_FLAGS_URG) printstr_log("TCP_FLAGS_URG\n");
			tcp_recv_pkt(i, pkt);
		}
	}
	set_rflags(rflags);
}
