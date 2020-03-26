#include "print.h"
#include "nasmfunc.h"
#include "int.h"
#include "dsctbl.h"

int *FONT_ADR;

void int_keyboard(int *esp) {
	unsigned char data;
	data = io_in8(0x0060);
	io_out8(0x20, 0x20);
	printnum(data);
	printstr("\n");
	return;
}

void kstart(void)
{
	init_gdtidt();
	init_pic();
	io_sti();
	initscreen();

	while(1){
		hlt();
	}
}
