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

#define AR_DATA64_RW	0x2092
// 1 00 1 0010 (0x92): system use,	readable, 	writable	, non executable

#define AR_CODE64_ER	0x209a
// 1 00 1 1010 (0x9a): system use, 	readable, 	non writable	, executable

#define AR_TSS32		0x0089

struct SEGMENT_DESCRIPTOR {
	uint16_t limit_low, base_low;
	uint8_t base_mid, access_right;
	uint8_t limit_high, base_high;
} __attribute__ ((packed));

struct IDTDescr {
   uint16_t target_lo; // offset bits 0..15
   uint16_t selector; // a code segment selector in GDT or LDT
   uint8_t ist;       // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
   uint8_t flags; // type and attributes
   uint16_t target_mid; // offset bits 16..31
   uint32_t target_hi; // offset bits 32..63
   uint32_t zero;     // reserved
} __attribute__ ((packed));

void init_gdtidt();
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_idt(uint8_t idtidx, void* handler);

#endif
