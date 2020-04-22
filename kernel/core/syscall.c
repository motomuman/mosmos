#include <stdint.h>
#include "dsctbl.h"
#include "print.h"
#include "intasm.h"

int syscall_handler(uint64_t rdi, uint64_t rsi)
{
	printstr_app("SYSCALL Handler\nrdi: ");
	printnum_app(rdi);
	printstr_app("\n");
	printstr_app("rsi: ");
	printstr_app((char*)rsi);
	printstr_app("\n");
	return 777;
}

void init_syscall()
{
	set_syscall(0x80, asm_syscall_handler);
}
