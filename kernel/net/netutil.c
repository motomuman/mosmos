#include <stdint.h>

uint16_t reverse16(uint16_t bytes) {
	uint16_t ret;
	ret = bytes>>8;
	ret |= (bytes&0xff) << 8;
	return ret;
}

uint16_t hton16(uint16_t bytes) {
	return reverse16(bytes);
}

uint16_t ntoh16(uint16_t bytes) {
	return reverse16(bytes);
}

uint32_t reverse32(uint32_t bytes)
{
	uint32_t ret;
	ret = 0;
	ret += bytes>>0 & 0xff;
	ret <<= 8;
	ret += bytes>>8 & 0xff;
	ret <<= 8;
	ret += bytes>>16 & 0xff;
	ret <<= 8;
	ret += bytes>>24 & 0xff;
	return ret;
}

uint32_t ntoh32(uint32_t bytes) {
	return reverse32(bytes);
}

uint32_t hton32(uint32_t bytes) {
	return reverse32(bytes);
}

uint16_t checksum(void * _data, uint16_t len)
{
	uint16_t *data = (uint16_t *) _data;
	int i;
	uint32_t sum = 0;
	for(i = 0; i < len/2; i++){
		sum += data[i];
		if(sum >> 16) {
			sum = (sum & 0xffff) + (sum>>16);
		}
	}

	if(len%2 == 1) {
		sum += ((uint8_t *)data)[len-1];
		if(sum >> 16) {
			sum = (sum & 0xffff) + (sum>>16);
		}
	}

	return (~sum) & 0xffff;
}
