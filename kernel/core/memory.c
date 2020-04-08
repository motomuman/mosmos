#include "nasmfunc.h"
#include "memory.h"
#include "print.h"
#include "lib.h"

#define MEM_TABLE_SIZE 2000

#define MAX_MEM_NAME 20

// 1M bytes
#define MEM_GRANULARITY 1024 * 1024

#define MEM_INIT_FLAG 0x11223344
#define MEM_FREE_FLAG 0x55667788
#define MEM_USED_FLAG 0xaabbccdd

struct memhdr {
	uint32_t mem_flag;
	char name[MAX_MEM_NAME + 1];
	uint32_t start;
	uint32_t size;
};

// maintain memory with 1MB granurarity
// TODO support flexible memory management
struct memtbl {
	uint32_t len;
	uint32_t addr[MEM_TABLE_SIZE];
};

struct memtbl freemem;

uint32_t memtest(unsigned int start, unsigned int end);

void mem_init() {
	// Check memory size(start: 0x00600000, end: 0xffffffff)
	uint32_t memstart = 0x00a00000;
	uint32_t memend = memtest(0x00a00000, 0xffffffff);

	freemem.len = 0;
	while(memstart + MEM_GRANULARITY <= memend && freemem.len < MEM_TABLE_SIZE) {
		freemem.addr[freemem.len] = memstart;
		struct memhdr *hdr = (struct memhdr *)memstart;
		hdr->mem_flag = MEM_FREE_FLAG;
		memset((uint8_t*)hdr->name, 0, MAX_MEM_NAME);
		hdr->start = memstart + sizeof(struct memhdr);
		hdr->size = MEM_GRANULARITY - sizeof(struct memhdr);

		freemem.len++;
		memstart += MEM_GRANULARITY;
	}
	return;
}

void mem_free(void *_addr) {
	uint32_t addr = (uint32_t) _addr;
	if(freemem.len >= MEM_TABLE_SIZE - 1) {
		printstr_log("INVALID MEM FREE ERROR\n");
		panic();
		return;
	}

	struct memhdr *entry = (struct memhdr *)(addr - sizeof(struct memhdr));
	if(entry->mem_flag != MEM_USED_FLAG) {
		printstr_log("Failed to free mem. Tried to free invalid address\n");
		panic();
		return;
	}
	entry->mem_flag = MEM_FREE_FLAG;

	printstr_log("Mem Free ");
	printstr_log(entry->name);
	printstr_log(" ");
	printhex_log(addr);
	printstr_log(" (");
	printnum_log(freemem.len);
	printstr_log(")");
	printstr_log("\n");

	freemem.addr[freemem.len] = addr - sizeof(struct memhdr);
	freemem.len++;
	return;
}

uint32_t mem_alloc(uint32_t size, char *name) {
	if (size > MEM_GRANULARITY) {
		printstr_log("ERROR: large memory req ");
		printstr_log(name);
		printstr_log("\n");
		panic();
		return 0;
	}

	if(freemem.len == 0) {
		printstr_log("ERROR: Don't have enough memory to allocate ");
		printstr_log(name);
		printstr_log("\n");
		panic();
		return 0;
	}

	freemem.len--;
	uint32_t addr = freemem.addr[freemem.len];
	struct memhdr *entry = (struct memhdr *)(addr);

	printstr_log("Mem Alloc ");
	printstr_log(name);
	printstr_log(" ");
	printhex_log(addr);
	printstr_log(" (");
	printnum_log(freemem.len);
	printstr_log(")");
	printstr_log("\n");

	if(entry->mem_flag != MEM_FREE_FLAG) {
		printstr_log("ERROR: Tried to allocate invalid address\n");
		panic();
		return 0;
	}
	entry->mem_flag = MEM_USED_FLAG;
	strncpy(entry->name, name, MAX_MEM_NAME);

	return entry->start;
}

int mem_free_size() {
	return freemem.len;
}

// 18th bit of eflags reg is AC flag.
// in 386 AC flag is always 0
#define EFLAGS_AC_BIT	 	0x00040000
#define CR0_CACHE_DISABLE 	0x60000000

// max of unsigned int is 4,294,967,295 (~=4GB)
uint32_t memtest(unsigned int start, unsigned int end)
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
