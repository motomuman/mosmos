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

#define NULL 0

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

struct tcp_socket {
	struct listctl rx_data_list;
	uint64_t wait_id;
	uint16_t sport;
	uint32_t sip;
	uint16_t dport;
	uint32_t dip;
	uint8_t flag;
	uint32_t seq_num;
	uint32_t ack_num;
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

void tcp_send(int socket_id, uint32_t dip, uint16_t dport, uint8_t *buf, uint32_t size, uint8_t flags);

void tcp_connect_timeout(void *_args)
{
	int socket_id = *(int*) _args;
	struct tcp_socket *socket = &tcp_sockets[socket_id];

	switch(socket->state) {
		case SYN_SENT:
			socket->state = CLOSED;
			printstr_app("tcp_state: SYN_SENT -> CLOSED\n");
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
			tcp_send(socket_id, socket->dip, socket->dport, NULL, 0, TCP_FLAGS_SYN);
			socket->seq_num++;
			socket->state = SYN_SENT;

			// set timeout
			int * args = (int *) mem_alloc(1 * sizeof(int), "tcp_timeout_arg");
			*args = socket_id;
			wq_push_with_delay(tcp_connect_timeout, args, 5000);

			printstr_app("tcp_state: CLOSED -> SYN_SENT\n");

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
			printstr_app("tcp_socket_connect, invalid state: ");
			printstr_app(get_tcp_state_str(socket->state));
			printstr_app("\n");
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
			printstr_app("tcp_socket_bind, invalid state: ");
			printstr_app(get_tcp_state_str(socket->state));
			printstr_app("\n");
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
			printstr_app("tcp_state: CLOSED -> LISTEN\n");
			return 0;
		case SYN_SENT:
		case CLOSE_WAIT:
		case LISTEN:
		case SYN_RCVD:
		case ESTABLISHED:
		case FIN_WAIT_1:
		case CLOSING:
			printstr_app("tcp_socket_listen, invalid state: ");
			printstr_app(get_tcp_state_str(socket->state));
			printstr_app("\n");
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
			printstr_app("tcp_socket_accept, invalid state: ");
			printstr_app(get_tcp_state_str(socket->state));
			printstr_app("\n");
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
			tcp_send(socket_id, socket->dip, socket->dport, NULL, 0, TCP_FLAGS_ACK | TCP_FLAGS_FIN);
			socket->seq_num++;
			socket->state = FIN_WAIT_1;
			printstr_app("tcp_state: ESTABLISHED -> FIN_WAIT_1\n");
			return 0;
		case CLOSED:
			return 0;
		case SYN_SENT:
		case FIN_WAIT_1:
		case CLOSING:
		case LISTEN:
		case SYN_RCVD:
		case CLOSE_WAIT:
			printstr_app("tcp_socket_close, invalid state: ");
			printstr_app(get_tcp_state_str(socket->state));
			printstr_app("\n");
			panic();
			return -1;
	}
	return -1;
}

void tcp_send(int socket_id, uint32_t dip, uint16_t dport, uint8_t *buf, uint32_t size, uint8_t flags)
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

	socket->seq_num += size;

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

	mem_free(txpkt->buf_head);
	mem_free(txpkt);
}

int tcp_socket_send(int socket_id, uint8_t *buf, int size)
{
	struct tcp_socket *socket = &tcp_sockets[socket_id];
	switch(socket->state) {
		case ESTABLISHED:
			tcp_send(socket_id, socket->dip, socket->dport, buf, size, TCP_FLAGS_ACK);
			return 0;
		case CLOSED:
		case SYN_SENT:
		case FIN_WAIT_1:
		case LISTEN:
		case SYN_RCVD:
		case CLOSE_WAIT:
		case CLOSING:
			printstr_app("tcp_socket_send, invalid state: ");
			printstr_app(get_tcp_state_str(socket->state));
			printstr_app("\n");
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

int tcp_socket_recv(int socket_id, uint8_t *buf, int size)
{
	if(socket_id < 0 || socket_id >= TCP_SOCKET_COUNT) {
		printstr_log("ERROR: invalid tcp socket_id\n");
		panic();
	}

	struct tcp_socket *socket = &tcp_sockets[socket_id];
	if(list_empty(&socket->rx_data_list)) {
		if(socket->state == ESTABLISHED) {
			// set timeout and sleep this task
			socket->wait_id = get_tick();
			int * args = (int *) mem_alloc(2 * sizeof(int), "tcp_timeout_arg");
			args[0] = socket_id;
			args[1] = socket->wait_id;
			wq_push_with_delay(tcp_socket_recv_timeout, args, 5000);
			task_sleep(socket);
		} else {
			return 0;
		}
	}

	//timeout
	if(list_empty(&socket->rx_data_list)) {
		return -1;
	}

	struct tcp_rx_data *rx_data = (struct tcp_rx_data *)list_popfront(&socket->rx_data_list);
	int data_len = min_int(size, rx_data->data_len);
	memcpy(buf, rx_data->data, data_len);
	return data_len;
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
				tcp_send(socket_id, socket->dip, socket->dport, NULL, 0, TCP_FLAGS_SYN | TCP_FLAGS_ACK);
				socket->seq_num++;
				socket->state = SYN_RCVD;
				printstr_app("tcp_state: LISTEN -> SYN_RCVD\n");
			}
			break;
		case SYN_RCVD:
			if((ntoh16(tcphdr->flags) & TCP_FLAGS_ACK)) {
				socket->state = ESTABLISHED;
				printstr_app("tcp_state: SYN_RCVD -> ESTABLISHED\n");
				task_wakeup(socket);
			}
			break;
		case SYN_SENT:
			if((ntoh16(tcphdr->flags) & TCP_FLAGS_SYN) && (ntoh16(tcphdr->flags) & TCP_FLAGS_ACK)) {
				socket->ack_num = ntoh32(tcphdr->seq_num) + 1;
				tcp_send(socket_id, socket->dip, socket->dport, NULL, 0, TCP_FLAGS_ACK);
				socket->state = ESTABLISHED;
				printstr_app("tcp_state: SYN_SENT -> ESTABLISHED\n");
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
				list_pushback(&socket->rx_data_list, &rx_data->link);
				task_wakeup(socket);
			}

			socket->ack_num = ntoh32(tcphdr->seq_num) + data_len;
			if((ntoh16(tcphdr->flags) & TCP_FLAGS_FIN) && (ntoh16(tcphdr->flags) & TCP_FLAGS_ACK)) {
				socket->ack_num++;
				tcp_send(socket_id, socket->dip, socket->dport, NULL, 0, TCP_FLAGS_ACK | TCP_FLAGS_FIN);
				socket->state = CLOSING;
				printstr_app("tcp_state: ESTABLISHED -> CLOSING\n");
			} else if (data_len > 0) {
				tcp_send(socket_id, socket->dip, socket->dport, NULL, 0, TCP_FLAGS_ACK);
			}
			break;
		case FIN_WAIT_1:
			if((ntoh16(tcphdr->flags) & TCP_FLAGS_FIN) && (ntoh16(tcphdr->flags) & TCP_FLAGS_ACK)) {
				socket->ack_num++;
				tcp_send(socket_id, socket->dip, socket->dport, NULL, 0, TCP_FLAGS_ACK);
				socket->state = CLOSED;
				printstr_app("tcp_state: FIN_WAIT_1 -> CLOSED\n");
			}
			break;
		case CLOSING:
			if((ntoh16(tcphdr->flags) & TCP_FLAGS_ACK)) {
				socket->state = CLOSED;
				printstr_app("tcp_state: CLOSING -> CLOSED\n");
			}
			break;
		default:
			break;
	}
}

void tcp_rx(struct pktbuf *pkt)
{
	struct tcp_hdr *tcphdr = (struct tcp_hdr *)pkt->buf;

	int i;
	uint64_t rflags = get_rflags();
	io_cli();

	for(i = 0; i < TCP_SOCKET_COUNT; i++){
		if(tcp_sockets[i].flag == TCP_SOCKET_USED && tcp_sockets[i].sport == ntoh16(tcphdr->dport)) {
			//if(ntoh16(tcphdr->flags) & TCP_FLAGS_FIN) printstr_app("TCP_FLAGS_FIN\n");
			//if(ntoh16(tcphdr->flags) & TCP_FLAGS_SYN) printstr_app("TCP_FLAGS_SYN\n");
			//if(ntoh16(tcphdr->flags) & TCP_FLAGS_RST) printstr_app("TCP_FLAGS_RST\n");
			//if(ntoh16(tcphdr->flags) & TCP_FLAGS_PSH) printstr_app("TCP_FLAGS_PSH\n");
			//if(ntoh16(tcphdr->flags) & TCP_FLAGS_ACK) printstr_app("TCP_FLAGS_ACK\n");
			//if(ntoh16(tcphdr->flags) & TCP_FLAGS_URG) printstr_app("TCP_FLAGS_URG\n");
			tcp_recv_pkt(i, pkt);
		}
	}
	set_rflags(rflags);
}
