void write();
void hlt();

void kstart(void)
{
	int i;
	char *p;

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
	for (i = 0; i <= 100000000; i++) {
	}

	for (i = 0xa0000; i <= 0xaffff; i += 1) {
		p = (char *) i;
		*p = 0x00;
	}

	write();

	for (i = 0; i <= 100000000; i++) {
	}

	for (i = 0xa0000; i <= 0xaffff; i += 1) {
		p = (char *) i;
		*p = 0x00;
	}

	for (i = 0; i <= 100000000; i++) {
	}

	write();


	for (i = 0xa0000; i <= 0xaffff; i += 8) {
		p = (char *) i;
		*p = 0x0f;
	}

	while(1){
		hlt();
	}
}
