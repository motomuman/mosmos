#ifndef _NASMFUNK_H_
#define _NASMFUNK_H_

int io_in8(int port);
int io_out8(int port, int data);
void io_cli();
void io_sti();
void io_hlt();
void io_stihlt();
int io_load_eflags();
void io_store_eflags(int eflags);
void load_idtr(int limit, int addr);
void load_gdtr(int limit, int addr);
void load_tr(int tr);
int load_cr0();
void store_cr0(int cr0);
void taskswitch3();
void taskswitch4();
void asm_int_keyboard();
void asm_int_pit();
unsigned int memtest_sub(unsigned int start, unsigned int end);

#endif
