#ifndef _NASMFUNK_H_
#define _NASMFUNK_H_

int io_in8(int port);
int io_out8(int port, int data);
void io_sti();
void hlt();
void load_idtr(int limit, int addr);

#endif
