#include "nasmfunc.h"
#include "memory.h"
#include "print.h"

#define MAX_MEMSIZE 2000

// maintain memory with 1MB granurarity
struct FREEMEM {
	int len;
	unsigned int addr[MAX_MEMSIZE];
};

struct FREEMEM mem;

void mem_init() {
	int i;
	mem.len = 0;
	for(i = 0; i < MAX_MEMSIZE; i++){
		mem.addr[i] = 0;
	}
	return;
}

void mem_free1m_batch(int start, int size) {
	int i;
	for(i = start; i + (1024 * 1024) <= start + size; i += (1024 * 1024)) {
		mem_free1m(i);
	}
	return;
}

void mem_free1m(int addr) {
	if(mem.len == MAX_MEMSIZE - 1) {
		return;
	}
	mem.addr[mem.len] = addr;
	mem.len++;
	return;
}

unsigned int mem_alloc1m() {
	if(mem.len == 0) {
		return 0;
	}
	mem.len--;
	return mem.addr[mem.len];
}

unsigned int mem_alloc(unsigned int request) {
	if (request > 1024 * 1024) {
		printstr("ERROR: large memory req\n");
		return 0;
	}
	return mem_alloc1m();
}

int mem_free_size() {
	return mem.len;
}

// 18th bit of eflags reg is AC flag.
// in 386 AC flag is always 0
#define EFLAGS_AC_BIT	 	0x00040000
#define CR0_CACHE_DISABLE 	0x60000000

// max of unsigned int is 4,294,967,295 (~=4GB)
unsigned int memtest(unsigned int start, unsigned int end)
{
	char flag486 = 0;
	unsigned int eflag, cr0, i;

	// Check 386 or 486
	// try to enable ac-bit
	eflag = io_load_eflags();
	eflag |= EFLAGS_AC_BIT; //enable ac bit
	io_store_eflags(eflag);

	// We can not enable ac bit in 386
	eflag = io_load_eflags();
	if((eflag & EFLAGS_AC_BIT) != 0) {
		flag486 = 1;
	}
	eflag &= ~EFLAGS_AC_BIT;
	io_store_eflags(eflag);

	// Disable cache
	// Cache was introduced to cpu equal to or after 486
	// memtest measure the memory size by
	// writing data to memory and reading data from memory
	// If cache is enabled, we can read data from address which exceeds physicall memory size
	if(flag486 != 0) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flag486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}

	return i;
}
