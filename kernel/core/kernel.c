#include <stdint.h>
#include "print.h"
#include "nasmfunc.h"
#include "int.h"
#include "dsctbl.h"
#include "timer.h"
#include "memory.h"
#include "task.h"
#include "r8169.h"
#include "workqueue.h"
#include "ether.h"
#include "netdev.h"
#include "arp.h"
#include "icmp.h"
#include "ip.h"
#include "raw.h"
#include "netutil.h"
#include "udp.h"

#define NULL 0

void print_key(void *_ch) {
	uint8_t *ch = (uint8_t *)_ch;
	printstr_app("print_key: ");
	printhex_app(*ch);
	printstr_app("\n");
	mem_free(ch);
}

void int_keyboard(int *esp) {
	uint8_t *ch = (uint8_t *)mem_alloc(1, "int_key_ch");
	pic_sendeoi(KEYBOARD_IRQ);
	*ch = io_in8(0x0060);	// get input key code
	printstr_log("keyint\n");
	wq_push(print_key, ch);
	return;
}

void task_ping_main() {
	int i;
	int ttl = 1;

	int sock = raw_socket(IP_HDR_PROTO_ICMP);

	uint8_t *buf = (uint8_t*) mem_alloc(sizeof(struct icmp_hdr) + sizeof(struct ip_hdr), "icmphdr");
	while(1) {
		for(i = 0; i < 100000000; i++){
		}
		struct icmp_hdr *icmphdr = (struct icmp_hdr *) buf;
		icmphdr->type = ICMP_HDR_TYPE_ECHO_REQUEST;
		icmphdr->code = 0;
		icmphdr->checksum = 0;
		icmphdr->echo.id = 7777;
		icmphdr->echo.seqnum = 0;

		//set checksum
		icmphdr->checksum = checksum(buf, sizeof(struct icmp_hdr));

		uint32_t dip = (8 << 24) | (8 << 16) | (8 << 8) | 8;
		printstr_app("task_b_send\n");
		raw_socket_send(sock, dip, buf, sizeof(struct icmp_hdr), ttl);

		int ret = raw_socket_recv(sock, buf, sizeof(struct icmp_hdr) + sizeof(struct ip_hdr));
		printstr_app("task_b_recv!!!!!\n");
		if(ret == -1) {
			printstr_app("TIMEOUT!!!!\n");
			continue;
		}
		printstr_app("TTL: ");
		printnum_app(ttl);
		printstr_app("\n");
		ttl++;
		struct ip_hdr *iphdr = (struct ip_hdr *)buf;
		printstr_app("src: ");
		printnum_app((ntoh32(iphdr->sip) >> 24) &0xff);
		printstr_app(".");
		printnum_app((ntoh32(iphdr->sip) >> 16) &0xff);
		printstr_app(".");
		printnum_app((ntoh32(iphdr->sip) >> 8) &0xff);
		printstr_app(".");
		printnum_app((ntoh32(iphdr->sip)) &0xff);
		printstr_app("\n");

		printstr_app("dst: ");
		printnum_app((ntoh32(iphdr->dip) >> 24) &0xff);
		printstr_app(".");
		printnum_app((ntoh32(iphdr->dip) >> 16) &0xff);
		printstr_app(".");
		printnum_app((ntoh32(iphdr->dip) >> 8) &0xff);
		printstr_app(".");
		printnum_app(ntoh32(iphdr->dip) &0xff);
		printstr_app("\n");


		buf += sizeof(struct ip_hdr);
		icmphdr = (struct icmp_hdr *)buf;
		if(icmphdr->type == ICMP_HDR_TYPE_ECHO_REPLY) {
			printstr_app("ICMP_HDR_TYPE_ECHO_REPLY\n");
		} else if(icmphdr->type == ICMP_HDR_TYPE_ECHO_REQUEST) {
			printstr_app("ICMP_HDR_TYPE_ECHO_REQUEST\n");
		} else if (icmphdr->type ==  ICMP_HDR_TYPE_TIME_EXCEEDED){
			printstr_app("ICMP_HDR_TYPE_TIME_EXCEEDED\n");
		}else{
			printstr_app("ICMP_HDR_TYPE_ECHO_INVALID\n");
		}
		printstr_app("CODE: ");
		printnum_app(icmphdr->code);
		printstr_app("\n");

		buf -= sizeof(struct ip_hdr);
	}
}

void task_b_main() {
	int i;
	int sock = udp_socket(IP_HDR_PROTO_ICMP);
	//uint32_t dip = (8 << 24) | (8 << 16) | (8 << 8) | 8;
	uint32_t dip = (192 << 24) | (168 << 16) | (2 << 8) | 1;
	uint8_t *buf = (uint8_t*) mem_alloc(10, "udpdata");
	//buf[0] = 'H';
	//buf[1] = 'i';
	//buf[2] = '!';
	//buf[3] = '!';
	//buf[4] = 0x0a;

	while(1) {
		for(i = 0; i < 200000000; i++){
		}
		printstr_app("task_b_main: send pkt\n");
		int ret = udp_socket_recv(sock, buf, 10);
		if(ret == -1) {
			printstr_app("udp_socket_recv: TIMEOUT\n");
			continue;
		}
		printstr_app("task_b_main: recv pkt\n");
		printstr_app("recv data: ");
		printstr_app(buf);
		printstr_app("\n");
	}
}

void task_c_main() {
	int i;
	while(1) {
		for(i = 0; i < 200000000; i++){
		}
		//printstr_app("task_c_main\n");
	}
}

void hello() {
	printstr_app("Hello Timer\n");
	set_timer(hello, NULL, 1000);
}

void hello2() {
	printstr_app("Hello Timer2\n");
	set_timer(hello2, NULL, 2000);
}


void kstart(void)
{
	initscreen();

	init_gdtidt();
	init_tss();
	tr_load();

	init_interrupt();
	mem_init();
	init_pit();
	init_timer();
	wq_init();

	init_arptable();
	init_r8169();
	//uint32_t ip_addr = (192 << 24) | (168 << 16) | (1 << 8) | 16;
	uint32_t ip_addr = (192 << 24) | (168 << 16) | (2 << 8) | 2;
	netdev_set_ip_addr(ip_addr);

	uint32_t gw_addr = (192 << 24) | (168 << 16) | (2 << 8) | 1;
	netdev_set_gw_addr(gw_addr);

	netdev_set_netmask(24);

	io_sti();

	//set_timer(hello, NULL, 1000);
	//set_timer(hello2, NULL, 3000);

	struct TASK *task_a;
	struct TASK *task_b;
	struct TASK *task_c;
	task_a = task_init();
	task_b = task_alloc(task_b_main);
	task_run(task_b);

	task_c = task_alloc(task_c_main);
	task_run(task_c);

	wq_set_receiver(task_a);

	while(1){
		io_cli();
		if(wq_empty()) {
			task_sleep();
			io_sti();
		} else {
			io_sti();
			wq_execute();
		}
	}
}
