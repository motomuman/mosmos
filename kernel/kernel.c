#include "print.h"
#include "nasmfunc.h"
#include "int.h"
#include "dsctbl.h"
#include "fifo.h"
#include "timer.h"

int *FONT_ADR;
struct FIFO8 keyfifo;

int current_task;
struct TSS32 {
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
};

void int_keyboard(int *esp) {
	unsigned char data;
	io_out8(0x20, 0x20);	// End of Interrupt command
	data = io_in8(0x0060);	// get input key code
	printstr("keyint\n");
	fifo8_put(&keyfifo, data);
	return;
}

int pitnum;
void int_pit(int *esp) {
	io_out8(0x20, 0x20);	// End of Interrupt command
	pitnum++;
	if(pitnum%200 == 0) {
		if(current_task == 3) {
			current_task = 4;
			taskswitch4();
		} else {
			current_task = 3;
			taskswitch3();
		}
		printstr("pit%200 = 0\n");
	}
	return;
}

void task_b_main() {
	int i;
	while(1) {
		for(i = 0; i < 20000000; i++){
		}
		printstr("task_b_main\n");
	}
}

void kstart(void)
{
	init_gdtidt();
	init_pic();
	io_sti();
	init_pit();
	initscreen();

	struct TSS32 tss_a, tss_b;

	tss_a.ldtr = 0;
	tss_a.iomap = 0x40000000;
	tss_b.ldtr = 0;
	tss_b.iomap = 0x40000000;

	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	// 103: limit (bytes of tss - 1)
	set_segmdesc(gdt + 3, 103, (int) &tss_a, AR_TSS32);
	set_segmdesc(gdt + 4, 103, (int) &tss_b, AR_TSS32);

	load_tr(3 * 8);	// Currently running on task3
	current_task = 3;

	tss_b.eip = (int) &task_b_main;
	tss_b.eflags = 0x00000202; /* IF = 1; */
	tss_b.eax = 0;
	tss_b.ecx = 0;
	tss_b.edx = 0;
	tss_b.ebx = 0;
	tss_b.esp  = 0x00080000;
	tss_b.ebp = 0;
	tss_b.esi = 0;
	tss_b.edi = 0;
	tss_b.es = 2 * 8;
	tss_b.cs = 1 * 8;
	tss_b.ss = 2 * 8;
	tss_b.ds = 2 * 8;
	tss_b.fs = 2 * 8;
	tss_b.gs = 2 * 8;

	unsigned char keybuf[32];
	fifo8_init(&keyfifo, 32, keybuf);

	int i;
	while(1){
		io_cli();
		if(fifo8_status(&keyfifo) == 0) {
			io_stihlt();
		} else {
			i = fifo8_get(&keyfifo);
			io_sti();
			printhex(i);
			printstr("\n");
		}
	}
}
