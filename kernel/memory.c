#include "nasmfunc.h"

// 18th bit of eflags reg is AC flag.
// in 386 AC flag is always 0
#define EFLAGS_AC_BIT	 	0x00040000
#define CR0_CACHE_DISABLE 	0x60000000

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
