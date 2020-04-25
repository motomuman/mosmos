#include <stdint.h>
#include "print.h"
#include "asm.h"

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

#define APPSCREEN_WIDTH 50

uint64_t FONT_ADR;

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
	appscreen.width = APPSCREEN_WIDTH;
	appscreen.height = 30;
	appscreen.x = 0;
	appscreen.y = 0;

	logscreen.startx = APPSCREEN_WIDTH + 1;
	logscreen.starty = 0;
	logscreen.width = 80 - APPSCREEN_WIDTH - 1;
	logscreen.height = 30;
	logscreen.x = 0;
	logscreen.y = 0;

	int width;
	uint64_t p;
	for(width = 0; width < 3; width++) {
		for (p = 0xa0000 + APPSCREEN_WIDTH + 80 * width; p <= 0xaffff; p += 80 * 16) {
			*(uint8_t *) p = 0x10;
		}
	}
}

void clear(struct SCREEN *screen) {
	uint64_t p;
	for (p = 0xa0000; p <= 0xaffff; p += 1) {
		int x = (p - 0xa0000) % 80;
		int y = (p - 0xa0000) / (80 * 16);
		if( x >= screen->startx && x < screen->startx + screen->width
				&& y >= screen->starty && y < screen->starty + screen->height ) {
			*(uint8_t *) p = 0x00;
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
	uint64_t rflags = get_rflags();
	io_cli();

	if(ch == '\n') {
		screen->x = 0;
		screen->y++;
	} else {
		int i;
		int y = screen->starty + screen->y;
		int x = screen->startx + screen->x;
		uint64_t vaddr = 0xa0000 + (80 * 16 * y) + x;
		uint8_t *fontaddr = (uint8_t *)(FONT_ADR + (ch<<4));

		for (i = 0; i < 16; i++){
			*(uint8_t *)vaddr = (*fontaddr);
			vaddr += 80;
			fontaddr += 1;
		}
		screen->x++;
	}
	fixpos(screen);

	set_rflags(rflags);
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

void printnum(struct SCREEN *screen, uint32_t num) {
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

void printhex(struct SCREEN *screen, uint64_t num) {
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

