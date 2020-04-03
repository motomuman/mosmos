#include "print.h"
#include "nasmfunc.h"
#include "int.h"
#include "dsctbl.h"
#include "fifo.h"
#include "timer.h"
#include "memory.h"
#include "task.h"
#include "r8169.h"

int *FONT_ADR;
struct FIFO8 keyfifo;

void int_keyboard(int *esp) {
	unsigned char data;
	io_out8(0x20, 0x20);	// End of Interrupt command
	data = io_in8(0x0060);	// get input key code
	printstr_log("keyint\n");
	fifo8_put(&keyfifo, data);
	return;
}

int pitnum;
void int_pit(int *esp) {
	io_out8(0x20, 0x20);	// End of Interrupt command
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
	init_gdtidt();
	init_pic();
	io_sti();
	init_pit();
	initscreen();

	init_r8169();

	// Check memory size(start: 0x00400000, end: 0xffffffff)
	int memsize = memtest(0x00400000, 0xffffffff);
	mem_init();
	mem_free1m_batch(0x00200000, memsize - 0x00200000);

	printstr_log("total memory: ");
	printnum_log(memsize/1024/1024);
	printstr_log(" MB\n");

	printstr_log("free memory: ");
	printnum_log(mem_free_size());
	printstr_log(" MB\n");

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

	unsigned char keybuf[32];
	fifo8_init(&keyfifo, 32, keybuf);
	keyfifo.receive_task = task_a;

	int i;
	while(1){
		//for(i = 0; i < 80000000; i++){
		//}
		//printstr("task_a_main\n");
		io_cli();
		if(fifo8_status(&keyfifo) == 0) {
			printstr_log("task sleep\n");
			task_sleep();
			io_sti();
		} else {
			i = fifo8_get(&keyfifo);
			io_sti();
			printhex_app(i);
			printstr_app("\n");
		}
	}
}
