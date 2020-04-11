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
	uint64_t start;
	uint64_t size;
};

// maintain memory with 1MB granurarity
// TODO support flexible memory management
struct memtbl {
	uint64_t len;
	uint64_t addr[MEM_TABLE_SIZE];
};

struct memtbl freemem;

void mem_init() {
	// Check memory size(start: 0x00600000, end: 0xffffffff)
	uint64_t memstart = 0x00a00000;
	// Hard code memory size
	// TODO Get memory size from somewhere
	//uint64_t memend = memtest(0x00a00000, 0xffffffff);
	uint64_t memend = 0x5DC00000; //1.5G

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
	uint64_t addr = (uint64_t) _addr;
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

uint64_t mem_alloc(uint32_t size, char *name) {
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
	uint64_t addr = freemem.addr[freemem.len];
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
