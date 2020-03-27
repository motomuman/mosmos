#include "print.h"

// width: 640 bit, height: 480 bit. Each row has 80 bytes (640 bits)
//
// 0x000A_0000 + (640/8) x 0 +------+------+- ... -+------+
// 0x000A_0000 + (640/8) x 1
// 0x000A_0000 + (640/8) x 2
//
// Each character has 8 bit width and 16 bit height.
// So we can have 80 characters in one line
// and can have 30 lines.
//
//                  +0     +1             +79
// + (80 x 16) x 0 +------+------+- ... -+------+
//                 |      |
//                 |      |
//                 |      |
//                 |      |
// + (80 x 16) x 1 +------+------+- ... -+------+
//                 |      |
//                 |      |
//                 |      |
//                 |      |
//

int FONT_ADR;
int x; int y;

void initscreen() {
	int tmpx = x;
	int tmpy = y;
	x = 0;
	y = 0;
	printnum(tmpx);
	printstr("\n");
	printnum(tmpy);
	printstr("\n");
}

void clear() {
	int i;
	char *p;

	for (i = 0xa0000; i <= 0xaffff; i += 1) {
		p = (char *) i;
		*p = 0x00;
	}
}

void fixpos() {
	if(x == 80){
		y++;
		x = 0;
	}
	if(y == 30){
		clear();
		y = 0;
		x = 0;
	}
}

void putchar(char ch) {
	if(ch == '\n') {
		x = 0;
		y++;
	} else {
		int i;
		int vaddr = 0xa0000 + (80 * 16 * y) + x;
		char *chaddr = (char *)(FONT_ADR + (ch<<4));

		for (i = 0; i < 16; i++){
			*(char *)vaddr = (*chaddr);
			vaddr += 80;
			chaddr += 1;
		}
		x++;
	}
	fixpos();
}

void printstr(char *str) {
	int i;
	for(i = 0; str[i] != 0; i++){
		putchar(str[i]);
	}
}

void printnum(int num) {
	if(num == 0) {
		putchar('0');
		return;
	}
	char buf[50];
	int len = 0;
	int neg = 0;
	if(num < 0) {
		neg = 1;
		num = -num;
	}
	while(num > 0){
		int digit = num % 10;
		buf[len] = '0' + digit;
		len++;
		num = num / 10;
	}
	len--;

	int i = 0;
	char printbuf[50];
	if(neg) {
		printbuf[0] = '-';
		i = 1;
	}
	for(; i <= len; i++){
		printbuf[i] = buf[len - i];
	}
	printbuf[i] = 0;
	printstr(printbuf);
}

void printhex(int num) {
	char buf[50];
	int len = 0;
	while(num > 0){
		int digit = num & 0x0f;
		if(digit < 10) {
			buf[len] = '0' + digit;
		} else {
			buf[len] = 'a' + digit - 10;
		}
		len++;
		num = num >> 4;
	}
	buf[len] = 'x';
	len++;
	buf[len] = '0';

	int i;
	char printbuf[50];
	for(i = 0; i <= len; i++){
		printbuf[i] = buf[len - i];
	}
	printbuf[i] = 0;
	printstr(printbuf);
}
