#include "nasmfunc.h"
#include "dsctbl.h"

struct gdtr {
	uint16_t size;
	uint64_t base;      /* (struct gdt_desc *) */
} __attribute__ ((packed));

struct idtr {
    uint16_t size;
    uint64_t base;      /* (struct idt_gate_desc *) */
} __attribute__ ((packed));

void init_gdtidt()
{
	int i;

	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	for (i = 0; i < GDT_NUM; i++) {
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_CODE64_ER);
	set_segmdesc(gdt + 2, 0xffffffff, 0x00000000, AR_DATA64_RW);

	struct gdtr *gdtr;
	gdtr = (struct gdtr *)(ADR_GDT + sizeof(struct SEGMENT_DESCRIPTOR) * GDT_NUM);
	gdtr->base = ADR_GDT;
	gdtr->size = GDT_NUM * sizeof(struct SEGMENT_DESCRIPTOR) - 1;
	load_gdtr(gdtr);

	// IDT
	for (i = 0; i < IDT_NUM; i++) {
		set_idt(i, 0);
	}

	struct idtr *idtr;
	idtr = (struct idtr *)(ADR_IDT + sizeof(struct IDTDescr) * IDT_NUM);
	idtr->base = ADR_IDT;
	idtr->size = IDT_NUM * sizeof(struct IDTDescr) - 1;
	load_idtr(idtr);
}

// Segment descriptor
//  63       |55  54  53  52  51        47  46 45|44 |    40|        |15
// +---------------------------------------------------------------------------+
// |         |   |   |   | A |         |   |     | D |      |        |         |
// |   base  | G | D | L | V |  limit  | P | DPL | T | Type |  Base  |  limit  |
// | [31-24] |   | B |   | L | [19-16] |   |     |   |      | [23-0] |  [15-0] |
// +---------------------------------------------------------------------------+
// base 	: memory range start
// limit	: memory range end
// G		: Granularity bit. If 1 the limit granularity is 4K
// DB		: If 1 32 bit segment
// L		: 1 for 64bit mode. 0 for others
// AVL		: any val
// P		: presence. If 1 keep on memory
// DPL		: Priviledge level
// DT		: memory segment
// Type		: Segment type
//
// ar
// xxxx0000xxxxxxxx
// +------------------------------------+
// |G|D|L|0| 0000 | P | DPL | DT | TYPE |
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
void set_idt_gatedesc(struct IDTDescr *idt, uint64_t base, uint16_t selector, uint8_t flags)
{
	idt->target_lo = (uint16_t)(base & 0xffffULL);
	idt->selector = (uint16_t)selector;
	idt->ist = 0;
	idt->flags = flags;
	idt->target_mid = (uint16_t)((base & 0xffff0000ULL) >> 16);
	idt->target_hi = (uint16_t)((base & 0xffffffff00000000ULL) >> 32);
	idt->zero = 0;
	return;
}

/* IDT flags */
#define IDT_PRESENT     0x80
#define IDT_INTGATE     0x0e

void set_idt(uint8_t idtidx, void* handler)
{
	struct IDTDescr *idt = (struct IDTDescr *) ADR_IDT;
	set_idt_gatedesc(idt + idtidx, (uint64_t) handler, 8, IDT_PRESENT | IDT_INTGATE);
}
