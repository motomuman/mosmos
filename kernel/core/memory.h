#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdint.h>

void mem_init();
void mem_free(void* _addr);
uint32_t mem_alloc(uint32_t size, char *name);
int mem_free_size();

#endif
