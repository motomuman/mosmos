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
#include "dns.h"
#include "lib.h"
#include "syscall.h"
#include "asm.h"
#include "user_app.h"
#include "keyboard.h"

#define NULL 0

void task_idle_main() {
	while(1) {
		io_hlt();
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
	init_keyboard();
	mem_init();
	init_pit();
	init_timer();
	init_syscall();
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


	//set_timer(hello, NULL, 1000);
	//set_timer(hello2, NULL, 3000);

	struct TASK *task_kernel;

	task_kernel = task_init();
	task_start(userland_main, TASK_PRIORITY_HIGH, 1);
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
