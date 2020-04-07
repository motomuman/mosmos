#include <stdint.h>

void memcpy(uint8_t *dst, uint8_t *src, uint32_t size)
{
	int i;
	for(i = 0; i < size; i++){
		dst[i] = src[i];
	}
}
