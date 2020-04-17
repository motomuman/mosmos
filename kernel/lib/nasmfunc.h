#ifndef _NASMFUNK_H_
#define _NASMFUNK_H_

#include <stdint.h>

uint8_t io_in8(uint16_t port);
int io_out8(uint16_t port, uint8_t data);
uint16_t io_in16(uint16_t port);
int io_out16(int port, int data);
uint32_t io_in32(uint16_t port);
void io_out32(uint16_t port, uint32_t data);
void io_cli();
void io_sti();
void io_hlt();
void io_stihlt();
void load_idtr(void *idtr);
void load_gdtr(void *gdtr);
void load_tr(uint16_t tr);
void farjmp(int cip, int cs);
void asm_int_keyboard();
void asm_int_pit();
void asm_int_r8169();
void task_switch();

#endif
