#include <stdint.h>
#include "print.h"
#include "nasmfunc.h"

void memcpy(uint8_t *dst, uint8_t *src, uint32_t size)
{
	int i;
	for(i = 0; i < size; i++){
		dst[i] = src[i];
	}
}

void memset(uint8_t *dst, uint8_t val, uint32_t size)
{
	int i;
	for(i = 0; i < size; i++){
		dst[i] = val;
	}
}

void strncpy(char *dst, char *src, uint32_t size)
{
	int i = 0;
	while(src[i] != 0 && i < size) {
		dst[i] = src[i];
		i++;
	}
}

void panic()
{
	printstr_log("PANIC");
	io_cli();
	while(1) {
		io_hlt();
	}
}
