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

uint32_t strlen(char *str)
{
	uint32_t len = 0;
	while(str[len] != 0) {
		len++;
	}
	return len;
}

void panic()
{
	printstr_log("PANIC");
	io_cli();
	while(1) {
		io_hlt();
	}
}

uint32_t max_uint32(uint32_t num1, uint32_t num2)
{
	return (num1 > num2) ? num1 : num2;
}

uint32_t min_uint32(uint32_t num1, uint32_t num2)
{
	return (num1 > num2) ? num2 : num1;
}
