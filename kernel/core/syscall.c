#include <stdint.h>
#include "dsctbl.h"
#include "nasmfunc.h"
#include "print.h"

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
	set_idt(0x40, asm_syscall_handler);
}
