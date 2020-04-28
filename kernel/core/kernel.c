#include <stdint.h>
#include "print.h"
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
#include "tcp.h"
#include "dns.h"
#include "lib.h"
#include "syscall.h"
#include "asm.h"
#include "user_app.h"
#include "keyboard.h"

void task_tcp_server() {
	int tcp_sock = tcp_socket();
	uint32_t my_addr = (192 << 24) | (168 << 16) | (2 << 8) | 2;
	int ret = tcp_socket_bind(tcp_sock, my_addr, 8888);
	int i;

	tcp_socket_listen(tcp_sock);
	ret = tcp_socket_accept(tcp_sock);
	if(ret == -1) {
		printstr_app("failed to accept\n");
	}
	tcp_socket_send(tcp_sock, (uint8_t *)"HELLO", 5);
	char buf[12];
	memset(buf, 0, 12);
	ret = tcp_socket_recv(tcp_sock, (uint8_t *)buf, 10);
	if(ret == -1) {
		printstr_app("tcp_socket_recv timeout\n");
	}

	printstr_app(buf);

	tcp_socket_close(tcp_sock);

	while(1) {
		for(i = 0; i < 1000000000; i++) {
		}
	}
}

void task_tcp_client() {
	int tcp_sock = tcp_socket();
	//uint32_t gw_addr = (192 << 24) | (168 << 16) | (2 << 8) | 1;
	uint32_t ip_addr = (216 << 24) | (58 << 16) | (199 << 8) | 238;
	int i;
	for(i = 0; i < 500000000; i++) {
	}
	int ret = tcp_socket_connect(tcp_sock, ip_addr, 80);
	if(ret != 1) {
		ret = tcp_socket_connect(tcp_sock, ip_addr, 80);
	}

	uint8_t buf[1500];
	memset(buf, 0, 1500);
	buf[0] = 'G';
	buf[1] = 'E';
	buf[2] = 'T';
	buf[3] = 0x20;
	buf[4] = '/';
	buf[5] = 0x20;
	buf[6] = 'H';
	buf[7] = 'T';
	buf[8] = 'T';
	buf[9] = 'P';
	buf[10] = '/';
	buf[11] = '1';
	buf[12] = '.';
	buf[13] = '1';
	buf[14] = 0x0d;
	buf[15] = 0x0a;
	buf[16] = 'H';
	buf[17] = 'o';
	buf[18] = 's';
	buf[19] = 't';
	buf[20] = ':';
	buf[21] = ' ';
	buf[22] = 'g';
	buf[23] = 'o';
	buf[24] = 'o';
	buf[25] = 'g';
	buf[26] = 'l';
	buf[27] = 'e';
	buf[28] = '.';
	buf[29] = 'c';
	buf[30] = 'o';
	buf[31] = 'm';
	buf[32] = 0x0d;
	buf[33] = 0x0a;
	buf[34] = 0x0d;
	buf[35] = 0x0a;

	tcp_socket_send(tcp_sock, buf, 36);

	memset(buf, 0, 1500);
	ret = tcp_socket_recv(tcp_sock, buf, 1500);
	if(ret == 0) {
		printstr_app("end read\n");
		tcp_socket_close(tcp_sock);
		goto end;
	} else if(ret == -1) {
		printstr_app("timeout\n");
		tcp_socket_close(tcp_sock);
		goto end;
	} else {
		printstr_app("has content\n");
	}
	printstr_app((char *)buf);
	printstr_app("\n");

	for(i = 0; i < 1000000000; i++) {
	}
	memset(buf, 0, 1500);
	ret = tcp_socket_recv(tcp_sock, buf, 1500);
	if(ret == 0) {
		printstr_app("end read\n");
		tcp_socket_close(tcp_sock);
		goto end;
	} else if(ret == -1) {
		printstr_app("timeout\n");
		tcp_socket_close(tcp_sock);
		goto end;
	} else {
		printstr_app("has content\n");
	}
	printstr_app((char *)buf);
	printstr_app("\n");

	for(i = 0; i < 1000000000; i++) {
	}
	memset(buf, 0, 1500);
	ret = tcp_socket_recv(tcp_sock, buf, 1500);
	if(ret == 0) {
		printstr_app("end read\n");
		tcp_socket_close(tcp_sock);
		goto end;
	} else if(ret == -1) {
		printstr_app("timeout\n");
		tcp_socket_close(tcp_sock);
		goto end;
	} else {
		printstr_app("has content\n");
	}
	printstr_app((char *)buf);
	printstr_app("\n");

	for(i = 0; i < 1000000000; i++) {
	}
	memset(buf, 0, 1500);
	ret = tcp_socket_recv(tcp_sock, buf, 1500);
	if(ret == 0) {
		printstr_app("end read\n");
		tcp_socket_close(tcp_sock);
		goto end;
	} else if(ret == -1) {
		printstr_app("timeout\n");
		tcp_socket_close(tcp_sock);
		goto end;
	} else {
		printstr_app("has content\n");
	}
	printstr_app((char *)buf);
	printstr_app("\n");

end:
	while(1) {
		for(i = 0; i < 1000000000; i++) {
		}
	}
}

void task_idle_main() {
	while(1) {
		io_hlt();
	}
}

void kstart(void)
{
	initscreen();

	init_gdtidt();
	init_tss();
	tr_load();

	init_interrupt();
	init_keyboard();
	mem_init();
	init_pit();
	init_timer();
	init_syscall();
	wq_init();

	udp_socket_init();
	tcp_socket_init();
	raw_socket_init();
	init_arptable();
	init_r8169();
	//uint32_t ip_addr = (192 << 24) | (168 << 16) | (1 << 8) | 16;
	uint32_t ip_addr = (192 << 24) | (168 << 16) | (2 << 8) | 2;
	netdev_set_ip_addr(ip_addr);

	//uint32_t gw_addr = (192 << 24) | (168 << 16) | (1 << 8) | 1;
	uint32_t gw_addr = (192 << 24) | (168 << 16) | (2 << 8) | 1;
	netdev_set_gw_addr(gw_addr);

	netdev_set_netmask(24);

	task_init();
	//task_start(userland_main, TASK_PRIORITY_HIGH, 1);
	task_start(task_tcp_server, TASK_PRIORITY_HIGH, 0);
	task_start(task_idle_main, TASK_PRIORITY_LOW, 0);

	io_sti();

	while(1){
		io_cli();
		if(wq_empty()) {
			task_sleep(wq_cond());
			io_sti();
		} else {
			io_sti();
			wq_execute();
		}
	}
}
