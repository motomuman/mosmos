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

int *FONT_ADR;

void int_keyboard(int *esp) {
	unsigned char data;
	pic_sendeoi(KEYBOARD_IRQ);
	data = io_in8(0x0060);	// get input key code
	printstr_log("keyint\n");
	struct work *w = (struct work *)mem_alloc(sizeof(struct work));
	w->type = wt_key_input;
	w->u.key_input.ch = data;
	wq_push(w);
	return;
}

int pitnum;
void int_pit(int *esp) {
	pic_sendeoi(PIT_IRQ);
	pitnum++;
	if(pitnum%200 == 0) {
		printstr_log("task_switch: ");
		task_show();
		task_switch();
	}
	return;
}

void task_b_main() {
	int i;
	while(1) {
		for(i = 0; i < 200000000; i++){
		}
		printstr_log("task_b_main\n");
	}
}

void task_c_main() {
	int i;
	while(1) {
		for(i = 0; i < 200000000; i++){
		}
		printstr_log("task_c_main\n");
	}
}

void kstart(void)
{
	initscreen();
	init_gdtidt();
	init_interrupt();
	mem_init();
	init_pit();
	wq_init();

	init_arptable();
	init_r8169();
	uint32_t ip_addr = (192 << 24) | (168 << 16) | (1 << 8) | 2;
	netdev_set_ip_addr(ip_addr);

	io_sti();

	struct TASK *task_a;
	struct TASK *task_b;
	struct TASK *task_c;
	task_a = task_init();
	task_b = task_alloc();
	task_b->tss.eip = (int) &task_b_main;
	task_run(task_b);

	task_c = task_alloc();
	task_c->tss.eip = (int) &task_c_main;
	task_run(task_c);

	wq_set_receiver(task_a);

	while(1){
		io_cli();
		if(wq_empty()) {
			printstr_log("task sleep\n");
			task_sleep();
			io_sti();
		} else {
			struct work * w = wq_pop();
			io_sti();
			switch(w->type) {
				case wt_key_input:
					printstr_app("wt_key_input: ");
					printhex_app(w->u.key_input.ch);
					printstr_app("\n");
					break;
				case wt_packet_receive:
					//printstr_app("wt_packet_receive\n");
					ether_rx(w->u.packet_receive.pbuf);
					mem_free1m((uint32_t)w->u.packet_receive.pbuf->buf_head);
					mem_free1m((uint32_t)w->u.packet_receive.pbuf);
					break;
			}
			mem_free1m((uint32_t)w);
		}
	}
}
