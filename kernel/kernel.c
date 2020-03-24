void kstart(void)
{
	int i;
	char *p;

	for (i = 0xa0000; i <= 0xaffff; i += 8) {
		p = (char *) i;
		*p = 0xff;
	}

	for (i = 0; i <= 1000000000; i++) {
	}
	for (i = 0xa0000; i <= 0xaffff; i += 8) {
		p = (char *) i;
		*p = 0x00;
	}

	for (i = 0; i <= 1000000000; i++) {
	}
	for (i = 0xa0000; i <= 0xaffff; i += 8) {
		p = (char *) i;
		*p = 0xf0;
	}

	for (i = 0; i <= 1000000000; i++) {
	}
	for (i = 0xa0000; i <= 0xaffff; i += 8) {
		p = (char *) i;
		*p = 0x0f;
	}

	while(1){
	}
}
