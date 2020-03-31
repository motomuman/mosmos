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
//         40         40         39
// 0                 39|41                79
// +-------------------+------------------+ 0
// |                   |                  |
// |                   |                  |
// |                   |                  |
// |                   |                  |
// |                   |                  |     30
// |                   |                  |
// |                   |                  |
// |                   |                  |
// |                   |                  |
// |                   |                  | 29
// +-------------------+------------------+

int FONT_ADR;

struct SCREEN {
	int width, height;
	int startx, starty;
	int x, y;
};

struct SCREEN appscreen;
struct SCREEN logscreen;
struct SCREEN monitorscreen;

void initscreen() {
	appscreen.startx = 0;
	appscreen.starty = 0;
	appscreen.width = 40;
	appscreen.height = 30;
	appscreen.x = 0;
	appscreen.y = 0;

	logscreen.startx = 41;
	logscreen.starty = 0;
	logscreen.width = 39;
	logscreen.height = 30;
	logscreen.x = 0;
	logscreen.y = 0;

	int i, width;
	char *p;
	for(width = 0; width < 3; width++) {
		for (i = 0xa0000 + 40 + 80 * width; i <= 0xaffff; i += 80 * 16) {
			p = (char *) i;
			*p = 0x10;
		}
	}
}

void clear(struct SCREEN *screen) {
	int i;
	char *p;

	for (i = 0xa0000; i <= 0xaffff; i += 1) {
		int x = (i - 0xa0000) % 80;
		int y = (i - 0xa0000) / (80 * 16);
		if( x >= screen->startx && x < screen->startx + screen->width
				&& y >= screen->starty && y < screen->starty + screen->height ) {
			p = (char *) i;
			*p = 0x00;
		}
	}
}

void fixpos(struct SCREEN *screen) {
	if(screen->x >= screen->width){
		screen->y++;
		screen->x = 0;
	}
	if(screen->y >= screen->height){
		clear(screen);
		screen->y = 0;
		screen->x = 0;
	}
}

void putchar(struct SCREEN *screen, char ch) {
	if(ch == '\n') {
		screen->x = 0;
		screen->y++;
	} else {
		int i;
		int y = screen->starty + screen->y;
		int x = screen->startx + screen->x;
		int vaddr = 0xa0000 + (80 * 16 * y) + x;
		char *chaddr = (char *)(FONT_ADR + (ch<<4));

		for (i = 0; i < 16; i++){
			*(char *)vaddr = (*chaddr);
			vaddr += 80;
			chaddr += 1;
		}
		screen->x++;
	}
	fixpos(screen);
}

void printstr(struct SCREEN *screen, char *str) {
	int i;
	for(i = 0; str[i] != 0; i++){
		putchar(screen, str[i]);
	}
}

void printstr_app(char *str) {
	printstr(&appscreen, str);
}

void printstr_log(char *str) {
	printstr(&logscreen, str);
}

void printnum(struct SCREEN *screen, int num) {
	if(num == 0) {
		putchar(screen, '0');
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
	printstr(screen, printbuf);
}

void printnum_app(int num) {
	printnum(&appscreen, num);
}

void printnum_log(int num) {
	printnum(&logscreen, num);
}

void printhex(struct SCREEN *screen, int num) {
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
	printstr(screen, printbuf);
}

void printhex_app(int num) {
	printhex(&appscreen, num);
}

void printhex_log(int num) {
	printhex(&logscreen, num);
}

