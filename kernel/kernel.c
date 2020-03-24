#include "print.h"

void write();
void hlt();

void kstart(void)
{
	int num = 0;
	while(1) {
		int i;
		for(i = 0; i < 500000000; i++){
		}
		printstr("hello world ");
		printnum(num);
		printstr("\n");
		num++;
	}

	while(1){
		hlt();
	}
}
