#include "nasmfunc.h"
#include "dsctbl.h"

void init_gdtidt()
{
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	struct GATE_DESCRIPTOR    *idt = (struct GATE_DESCRIPTOR    *) ADR_IDT;
	int i;

	for (i = 0; i < LIMIT_GDT / 8; i++) {
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_CODE32_ER);
	set_segmdesc(gdt + 2, 0xffffffff, 0x00000000, AR_DATA32_RW);
	load_gdtr(LIMIT_GDT, ADR_GDT);

	for (i = 0; i <= LIMIT_IDT / 8; i++) {
		set_gatedesc(idt + i, 0, 0, 0);
	}
	load_idtr(LIMIT_IDT, ADR_IDT);

	set_gatedesc(idt + 0x21, (int) asm_int_keyboard, 8, 0x008e);
}

// Segment descriptor
//  63       |55  54  53  52  51        47  46 45|44 |    40|        |15
// +---------------------------------------------------------------------------+
// |         |   |   |   | A |         |   |     | D |      |        |         |
// |   base  | G | D | 0 | V |  limit  | P | DPL | T | Type |  Base  |  limit  |
// | [31-24] |   |   |   | L | [19-16] |   |     |   |      | [23-0] |  [15-0] |
// +---------------------------------------------------------------------------+
// base 	: memory range start
// limit	: memory range end
// G		: Granularity bit. If 1 the limit granularity is 4K
// D		: If 1 32 bit segment
// AVL		: any val
// P		: presence. If 1 keep on memory
// DPL		: Priviledge level
// DT		: memory segment
// Type		: Segment type
//
// ar
// xxxx0000xxxxxxxx
// +------------------------------------+
// |G|D|0|0| 0000 | P | DPL | DT | TYPE |
// +------------------------------------+
// example of lower 1 byte
// 0 00 0 0000 (0x00): unused descriptor table
// 1 00 1 0010 (0x92): system use,	readable, 	writable	, non executable
// 1 00 1 1010 (0x9a): system use, 	readable, 	non writable	, executable
// 1 11 1 0010 (0xf2): application use, readable, 	writable	, non executable
// 1 11 1 1010 (0xfa): application use, readable, 	non writable	, executable
//
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	if (limit > 0xfffff) {
		ar |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;
	}
	sd->limit_low    = limit & 0xffff;
	sd->base_low     = base & 0xffff;
	sd->base_mid     = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high    = (base >> 24) & 0xff;
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
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low   = offset & 0xffff; gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	return;
}
