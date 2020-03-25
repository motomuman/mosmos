#include "print.h"

void write();
void hlt();
int *FONT_ADR;

void kstart(void)
{
	initscreen();
	printstr("hello\n");
	printnum((int)FONT_ADR);
	printstr("\n");
	printnum(*FONT_ADR);
	printstr("\n");
	printnum((*(int*)0x00100000));
	printstr("\n");
	//int num = 0;
	//while(1) {
	//	int i;
	//	for(i = 0; i < 500000000; i++){
	//	}
	//	printstr("hello world ");
	//	printnum(num);
	//	printstr("\n");
	//	num++;
	//}

	while(1){
		hlt();
	}
}
