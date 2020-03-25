#include "print.h"
#include "nasmfunk.h"

void hlt();
int *FONT_ADR;

void int_keyboard(int *esp) {
	char data;
	data = io_in8(0x0060);
	io_out8(0x20, 0x20);
	printnum(data);
	printstr("\n");
	return;
}

void kstart(void)
{
	initscreen();

	while(1){
		hlt();
	}
}
