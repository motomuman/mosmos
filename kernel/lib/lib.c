#include <stdint.h>
#include "print.h"
#include "nasmfunc.h"

void memcpy(void *_dst, void *_src, uint32_t size)
{
	uint8_t *dst = (uint8_t *)_dst;
	uint8_t *src = (uint8_t *)_src;
	int i;
	for(i = 0; i < size; i++){
		dst[i] = src[i];
	}
}

void memset(void *_dst, uint8_t val, uint32_t size)
{
	uint8_t *dst = (uint8_t *)_dst;
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
