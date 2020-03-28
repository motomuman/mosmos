#ifndef _MEMORY_H_
#define _MEMORY_H_

void mem_init();
void mem_free1m_batch(int start, int size);
void mem_free1m(int addr);
unsigned int mem_alloc1m();
int mem_free_size();

unsigned int memtest(unsigned int start, unsigned int end);

#endif
