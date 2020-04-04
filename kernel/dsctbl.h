#ifndef _DSCTBL_H_
#define _DSCTBL_H_

#include <stdint.h>

//IDT 0x00100000 - 0x001007ff
#define ADR_IDT 0x00100000

// 0x7ff (0b11111111111) (2047) = 256 * 8 - 1
#define LIMIT_IDT 0x000007ff

//GDT 0x00090000 - 0x0009ffff
#define ADR_GDT 0x00090000

// 0xffff (65535) 
#define LIMIT_GDT 0x0000ffff

#define AR_DATA32_RW	0x4092
// 1 00 1 0010 (0x92): system use,	readable, 	writable	, non executable

#define AR_CODE32_ER	0x409a
// 1 00 1 1010 (0x9a): system use, 	readable, 	non writable	, executable

#define AR_TSS32		0x0089

struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};

void init_gdtidt();
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_idt(uint8_t idtidx, uint32_t handler);

#endif
