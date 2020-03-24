void write();
void hlt();

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
int putchar(int x, int y, char ch) {
	if (y < 0 || y >= 30 || x < 0 || x >= 80) {
		return -1;
	}
	int i;
	int vaddr = 0xa0000 + (80 * 16 * y) + x;
	char *chaddr = (char *)((*(int *)0x00100000) + (ch<<4));

	for (i = 0; i < 16; i++){
		*(char *)vaddr = (*chaddr);
		vaddr += 80;
		chaddr += 1;
	}
	return 0;
}

void kstart(void)
{
	int i;
	char *p;

	for (i = 0xa0000; i <= 0xaffff; i += 1) {
		p = (char *) i;
		*p = 0x00;
	}

	int x = 0;
	int y = 0;
	int ch;
	for(ch = 0; ch < 255; ch++){
		putchar(x, y, (char)ch);
		x++;
		if(x == 80){
			y++;
			x = 0;
		}
	}

	putchar(33, 14, 'H');
	putchar(34, 14, 'e');
	putchar(35, 14, 'l');
	putchar(36, 14, 'l');
	putchar(37, 14, 'o');
	putchar(38, 14, ' ');
	putchar(39, 14, 'M');
	putchar(40, 14, 'O');
	putchar(41, 14, 'S');
	putchar(42, 14, 'M');
	putchar(43, 14, 'O');
	putchar(44, 14, 'S');
	putchar(45, 14, 'S');
	putchar(46, 14, '!');

	while(1){
		hlt();
	}
}
