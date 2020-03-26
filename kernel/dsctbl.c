#include "nasmfunc.h"

// 0x7ff (0b11111111111) (2047) = 256 * 8 - 1
#define LIMIT_IDT 0x000007ff
#define ADR_IDT 0x00100000

struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};

void asm_int_keyboard();
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);

void init_gdtidt()
{
	struct GATE_DESCRIPTOR    *idt = (struct GATE_DESCRIPTOR    *) ADR_IDT;

	int i;
	for (i = 0; i <= LIMIT_IDT / 8; i++) {
		set_gatedesc(idt + i, 0, 0, 0);
	}
	load_idtr(LIMIT_IDT, ADR_IDT);

	set_gatedesc(idt + 0x21, (int) asm_int_keyboard, 8, 0x008e);
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	return;
}
// Interruption gate descriptor
//63                  |47             45             40       32|                  |15          0
// ----------------------------------------------------------------------------------+
// |    offset (H)    | P(1) | DPL(00) | DT(0) | Type |        | Segment selector | offset (L) |
// ----------------------------------------------------------------------------------+
// int_handler[31:16] |    8(1000)             |   E  |  0 | 0 | 0 | 0 | 0 |  8   | int_handler[15:0]
//
// offset: 		interruption handler
// segment selector:	select segment to use
// P:			presense (exists on memory)
// DPL:			Descriptor Privilege Level
// DT:			0 for gate descriptor?
// TYPE:			0xE for 386 interruption gate
