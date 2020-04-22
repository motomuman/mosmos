#ifndef _DSCTBL_H_
#define _DSCTBL_H_

#include <stdint.h>

void init_gdtidt();
void set_idt(uint8_t idtidx, void* handler);
void set_syscall(uint8_t idtidx, void* handler);
void init_tss(void);
void tr_load();

#endif
