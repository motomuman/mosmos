#ifndef _DSCTBL_H_
#define _DSCTBL_H_

#include <stdint.h>

//IDT 0x00100000 - 0x001007ff
#define ADR_IDT 0x00100000

#define IDT_NUM 100

//GDT 0x00090000 - 0x0009ffff
#define ADR_GDT 0x00090000

// 0xffff (65535) 
// #define LIMIT_GDT 0x0000ffff
#define GDT_NUM 128

struct gdt_seg_desc;

void init_gdtidt();
void set_idt(uint8_t idtidx, void* handler);
void init_tss(void);
void tr_load();

#endif
