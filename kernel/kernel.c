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
	pic_sendeoi(KEYBOARD_IRQ);
	data = io_in8(0x0060);	// get input key code
	printstr_log("keyint\n");
	fifo8_put(&keyfifo, data);
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
	init_r8169();
	io_sti();

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
