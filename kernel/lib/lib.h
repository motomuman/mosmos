#ifndef _LIB_H_
#define _LIB_H_

void memcpy(uint8_t *dst, uint8_t *src, uint32_t size);
void memset(uint8_t *dst, uint8_t val, uint32_t size);
void strncpy(char *dst, char *src, uint32_t size);
void panic();

#endif
