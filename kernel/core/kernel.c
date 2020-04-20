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
#include "dns.h"
#include "lib.h"

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

	int udp_sock = udp_socket();

	uint8_t *buf = (uint8_t*) mem_alloc(sizeof(struct icmp_hdr) + sizeof(struct ip_hdr), "icmphdr");
	while(1) {
		for(i = 0; i < 100000000; i++){
		}
		int sock = raw_socket(IP_HDR_PROTO_ICMP);

		struct icmp_hdr *icmphdr = (struct icmp_hdr *) buf;
		icmphdr->type = ICMP_HDR_TYPE_ECHO_REQUEST;
		icmphdr->code = 0;
		icmphdr->checksum = 0;
		icmphdr->echo.id = 7777;
		icmphdr->echo.seqnum = 0;

		//set checksum
		icmphdr->checksum = checksum(buf, sizeof(struct icmp_hdr));

		uint32_t dip = (8 << 24) | (8 << 16) | (8 << 8) | 8;
		raw_socket_send(sock, dip, buf, sizeof(struct icmp_hdr), ttl);

		int ret = raw_socket_recv(sock, buf, sizeof(struct icmp_hdr) + sizeof(struct ip_hdr));
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

		char domainbuf[200];
		ret = resolve_host(udp_sock, ntoh32(iphdr->sip) , domainbuf, 200);
		if(ret != -1){
			printstr_app(" (");
			printstr_app(domainbuf);
			printstr_app(")\n");
		} else {
			printstr_app("\n");
		}

		//printstr_app("dst: ");
		//printnum_app((ntoh32(iphdr->dip) >> 24) &0xff);
		//printstr_app(".");
		//printnum_app((ntoh32(iphdr->dip) >> 16) &0xff);
		//printstr_app(".");
		//printnum_app((ntoh32(iphdr->dip) >> 8) &0xff);
		//printstr_app(".");
		//printnum_app(ntoh32(iphdr->dip) &0xff);
		//printstr_app("\n");


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

		buf -= sizeof(struct ip_hdr);

		raw_socket_free(sock);
	}
}

void task_b_main() {
	int i;
	int sock = udp_socket();
	char buf[200];

	while(1) {
		for(i = 0; i < 200000000; i++){
		}

		uint32_t ip = resolve_addr(sock, "www.hongo.wide.ad.jp");
		if(ip != 0) {
			printstr_app("task_b: resolved ip ");
			printnum_app((ip>>24)&0xff);
			printstr_app(".");
			printnum_app((ip>>16)&0xff);
			printstr_app(".");
			printnum_app((ip>>8)&0xff);
			printstr_app(".");
			printnum_app((ip>>0)&0xff);
			printstr_app("\n");
		}

		memset(buf, 0, 100);
		int ret = resolve_host(sock, ((203 << 24) + (178 << 16) + (135 << 8) + 39), buf, 200);
		if(ret != -1){
			printstr_app("task_b: resolved host ");
			printstr_app(buf);
			printstr_app("\n");
		}
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

	udp_socket_init();
	raw_socket_init();
	init_arptable();
	init_r8169();
	uint32_t ip_addr = (192 << 24) | (168 << 16) | (1 << 8) | 16;
	//uint32_t ip_addr = (192 << 24) | (168 << 16) | (2 << 8) | 2;
	netdev_set_ip_addr(ip_addr);

	uint32_t gw_addr = (192 << 24) | (168 << 16) | (1 << 8) | 1;
	//uint32_t gw_addr = (192 << 24) | (168 << 16) | (2 << 8) | 1;
	netdev_set_gw_addr(gw_addr);

	netdev_set_netmask(24);

	io_sti();

	//set_timer(hello, NULL, 1000);
	//set_timer(hello2, NULL, 3000);

	struct TASK *task_a;
	struct TASK *task_b;
	struct TASK *task_c;
	task_a = task_init();
	task_b = task_alloc(task_ping_main);
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
