#ifndef _LIB_H_
#define _LIB_H_

void memcpy(void *_dst, void *_src, uint32_t size);
void memset(void *_dst, uint8_t val, uint32_t size);
void strncpy(char *dst, char *src, uint32_t size);
void panic();

#endif
