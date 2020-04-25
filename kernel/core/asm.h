#ifndef _ASM_H_
#define _ASM_H_

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
void asm_int_keyboard();
void asm_int_pit();
void asm_int_r8169();
void asm_syscall_handler();
uint64_t test_and_set(uint64_t *mtx, uint64_t val);
uint64_t get_rflags();
void task_switch();

void asm_int_0();
void asm_int_1();
void asm_int_2();
void asm_int_3();
void asm_int_4();
void asm_int_5();
void asm_int_6();
void asm_int_7();
void asm_int_8();
void asm_int_9();
void asm_int_10();
void asm_int_11();
void asm_int_12();
void asm_int_13();
void asm_int_14();
void asm_int_15();
void asm_int_16();
void asm_int_17();
void asm_int_18();
void asm_int_19();

#endif
