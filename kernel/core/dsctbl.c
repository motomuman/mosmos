#include "nasmfunc.h"
#include "dsctbl.h"
#include "lib.h"

// 64bit tss
struct tss {
    uint32_t reserved1;
    uint32_t rsp0l;
    uint32_t rsp0h;
    uint32_t rsp1l;
    uint32_t rsp1h;
    uint32_t rsp2l;
    uint32_t rsp2h;
    uint32_t reserved2;
    uint32_t reserved3;
    uint32_t ist1l;
    uint32_t ist1h;
    uint32_t ist2l;
    uint32_t ist2h;
    uint32_t ist3l;
    uint32_t ist3h;
    uint32_t ist4l;
    uint32_t ist4h;
    uint32_t ist5l;
    uint32_t ist5h;
    uint32_t ist6l;
    uint32_t ist6h;
    uint32_t ist7l;
    uint32_t ist7h;
    uint32_t reserved4;
    uint32_t reserved5;
    uint16_t reserved6;
    uint16_t iomap;
} __attribute__ ((packed));

#define DESC_TYPE_TSS_AVAILABLE    0x9

struct gdt_tss_desc {
    uint16_t limit_low;
    uint16_t base_low;
    uint16_t w2;
    uint16_t w3;
    uint32_t base_high;
    uint32_t reserved;
} __attribute__ ((packed));

/*
 *                    64-bit TSS Descriptor
 *
 * 31                      13 12             8 7                0
 * +-------------------------------------------------------------+
 * |     reserved            |       0        |     reserved     | 12
 * +-------------------------------------------------------------+
 *
 * 31                                                           0
 * +-------------------------------------------------------------+
 * |                         base [63-32]                        | 8
 * +-------------------------------------------------------------+
 *
 *  31     24 23 22 21 20 19    16 15 14  13 12 11    8 7       0
 * +-------------------------------------------------------------+
 * |         |  |  |  |A |         |  |  D  |   |      |         |
 * |  base   |G |0 |0 |V |  limit  |p |  P  | 0 | type |  base   | 4
 * | [31-24] |  |  |  |L | [19-16] |  |  L  |   |      | [23-16] |
 * +-------------------------------------------------------------+
 *
 * +-------------------------------------------------------------+
 * |          base [15-0]         |          limit [15-0]        | 0
 * +-------------------------------------------------------------+
 *
 * AVL:	Available for use by system software
 * B:	Busy flag
 * BASE:Segment Base Address
 * DPL:	Descriptor Privilege Level
 * G:	Granularity
 * LIMIG: Segment Limit
 * P:	Segment Present
 * TYPE: Segment Type
 *
 */

/* GDT type */
#define GDT_SEG_TYPE_EX     1<<3
#define GDT_SEG_TYPE_DC     1<<2
#define GDT_SEG_TYPE_RW     1<<1
#define GDT_SEG_TYPE_AC     1<<0

struct gdt_seg_desc {
	uint16_t limit_low;
	uint16_t base_low;
	uint16_t w2;
	uint16_t w3;
} __attribute__ ((packed));

/*
 *                     Segment descriptor
 *
 *  31     24 23  22  21  20  19    16  15  14 13 12  11   8 7         0
 * +--------------------------------------------------------------------+
 * |         |   |   |   | A |         |   |  D  |   |      |           |
 * |   base  | G | D | L | V |  limit  | P |  P  | S | Type |   Base    | 4
 * | [31-24] |   | B |   | L | [19-16] |   |  L  |   |      |  [23-16]  |
 * +--------------------------------------------------------------------+
 *  31                               16 15                             0
 * +--------------------------------------------------------------------+
 * |          base [15 - 0]            |        limit [15 - 0]          | 0
 * +--------------------------------------------------------------------+
 *
 * base 	: memory range start
 * limit	: memory range end
 * G		: Granularity bit. If 1 the limit granularity is 4K
 * DB		: If 1 32 bit segment
 * L		: 1 for 64bit mode. 0 for others
 * AVL		: Any Value (Available for use by system software)
 * P		: Segment presence. If 1 keep on memory
 * DPL		: Descriptor Priviledge level
 * S		: Descriptor type (0 = system; 1 = code or data)
 * Type		: Segment type
 *
 *           EWA
 * Type	0  (0000) Read-Only
 * 	1  (0001) Read-Only, Accessed
 * 	2  (0010) Read/Write
 * 	3  (0011) Read/Write, Accessed
 * 	4  (0100) Read-only, expand-down
 * 	5  (0101) Read-only, expand-down, Accessed
 * 	6  (0110) Read/Write,expand-down
 * 	7  (0111) Read/Write,expand-down, Accessed

 *           CRA
 * 	8  (1000) Execute-Only
 * 	9  (1001) Execute-Only, Accessed
 * 	10 (1010) Execute/Read
 * 	11 (1011) Execute/Read, Accessed
 * 	12 (1100) Execute-Only, conforming
 * 	13 (1101) Execute-Only, conforming, Accessed
 * 	14 (1110) Execute/Read-only, conforming
 *  	15 (1111) Execute/Read-only, conforming, Accessed
 */

struct gdtr {
	uint16_t size;
	uint64_t base;
} __attribute__ ((packed));

#define IDT_GATE_FLAG_PRESENT     0x80
#define IDT_GATE_FLAG_INTGATE     0x0e
#define IDT_GATE_FLAG_DPL_USER    0x60

struct idt_gate {
   uint16_t offset_low;
   uint16_t selector;
   uint8_t ist;
   uint8_t flags;
   uint16_t offset_mid;
   uint32_t offset_hi;
   uint32_t zero;
} __attribute__ ((packed));

/*                      Interruption gate
 * 31                                                          0
 * +-----------------------------------------------------------+
 * |                        reserved                           | 12
 * +-----------------------------------------------------------+
 *
 * 31                                                          0
 * +-----------------------------------------------------------+
 * |                       offset [63-32]                      | 8
 * +-----------------------------------------------------------+
 *
 * 31                  16 15 14 13 12  11     8 7     3  2     0
 * +-----------------------------------------------------------+
 * |                     |  |     |   |        |        |      |
 * |       offset        |P | DPL | 0 |  type  |    0   |  IST | 4
 * |      [31-24]        |  |     |   |        |        |      |
 * +-----------------------------------------------------------+
 *
 * 31                           16 15                          0
 * +-----------------------------------------------------------+
 * |      Segment Selector        |        offset [15-0]       | 0
 * +-----------------------------------------------------------+
 *
 * DPL:		Descriptor Privilege Level
 * Offset:	Offset to procedure entry point
 * P:		Segment Present flag
 * Selector	Segment Selector for destination code segment
 * IST:		Interrupt Stack Table
 */

struct idtr {
    uint16_t size;
    uint64_t base;
} __attribute__ ((packed));

struct tss tss_storage;

void set_gdt_tss_desc(struct gdt_tss_desc *e, uint64_t base, uint32_t limit,
		uint8_t type, uint8_t dpl, uint8_t g);
void set_gdt_seg_desc(struct gdt_seg_desc *sd, uint64_t base, uint64_t limit,
		uint8_t type, uint8_t dpl, uint8_t l, uint8_t db, uint8_t g);

/*
 * GDT 0x00090000 - 0x0009ffff
 *
 * struct gdt_seg_desc: 8bytes
 * struct gdt_tss_desc: 16bytes
 * struct gdtr: 10bytes
 * 3 * 8 + 16 + 10 = 50bytes
 */
#define ADR_GDT 0x00090000
#define GDT_TSS_START 5

/*
 * IDT 0x00100000 - 0x001007ff
 *
 * struct idt_gate: 16bytes
 * struct idtr: 10bytes
 * 16 * 256 + 10 = 4106 (4KB + 10B)
 */
#define ADR_IDT 0x00091000
#define IDT_NUM 256

void init_gdtidt()
{
	int i;

	struct gdt_seg_desc *gdt = (struct gdt_seg_desc *) ADR_GDT;

	// Null descriptor
	memset(gdt, 0, sizeof(struct gdt_seg_desc));

	uint8_t code = GDT_SEG_TYPE_EX | GDT_SEG_TYPE_RW;
	uint8_t data = GDT_SEG_TYPE_RW;

	// Code/Data descriptor
	//set_gdt_seg_desc(*sd,      base,       limit,   type, dpl, l, db, g)
	set_gdt_seg_desc(gdt + 1, 0x00000000, 0xffffffff, code,   0, 1, 0, 1); //kernel code
	set_gdt_seg_desc(gdt + 2, 0x00000000, 0xffffffff, data,   0, 1, 0, 1); //kernet data
	set_gdt_seg_desc(gdt + 3, 0x00000000, 0xffffffff, code,   3, 1, 0, 1); //user code
	set_gdt_seg_desc(gdt + 4, 0x00000000, 0xffffffff, data,   3, 1, 0, 1); //user data

	// tss
	struct gdt_tss_desc *tss;
	tss = (struct gdt_tss_desc *)(ADR_GDT + GDT_TSS_START * sizeof(struct gdt_seg_desc));
	set_gdt_tss_desc(tss, (uint64_t) &tss_storage, sizeof(struct tss) - 1, DESC_TYPE_TSS_AVAILABLE, 0, 0);

	// gdtr
	struct gdtr *gdtr;
	uint64_t sz = GDT_TSS_START * sizeof(struct gdt_seg_desc) + 1 * sizeof(struct gdt_tss_desc);
	gdtr = (struct gdtr *)(ADR_GDT + sz);
	gdtr->base = ADR_GDT;
	gdtr->size = sz - 1;
	load_gdtr(gdtr);

	// IDT
	for (i = 0; i < IDT_NUM; i++) {
		set_idt(i, 0);
	}
	set_idt(0, asm_int_0);
	set_idt(1, asm_int_1);
	set_idt(2, asm_int_2);
	set_idt(3, asm_int_3);
	set_idt(4, asm_int_4);
	set_idt(5, asm_int_5);
	set_idt(6, asm_int_6);
	set_idt(7, asm_int_7);
	set_idt(8, asm_int_8);
	set_idt(9, asm_int_9);
	set_idt(10, asm_int_10);
	set_idt(11, asm_int_11);
	set_idt(12, asm_int_12);
	set_idt(13, asm_int_13);
	set_idt(14, asm_int_14);
	set_idt(15, asm_int_15);
	set_idt(16, asm_int_16);
	set_idt(17, asm_int_17);
	set_idt(18, asm_int_18);
	set_idt(19, asm_int_19);

	struct idtr *idtr;
	idtr = (struct idtr *)(ADR_IDT + sizeof(struct idt_gate) * IDT_NUM);
	idtr->base = ADR_IDT;
	idtr->size = IDT_NUM * sizeof(struct idt_gate) - 1;
	load_idtr(idtr);
}

void init_tss(void)
{
	struct tss *tss = &tss_storage;
	memset(tss, 0, sizeof(struct tss));
	tss->rsp0l = 0x00010000;
	tss->rsp0h = 0;
}

void tr_load()
{
    load_tr(GDT_TSS_START * sizeof(struct gdt_seg_desc));
}

void set_gdt_seg_desc(struct gdt_seg_desc *sd, uint64_t base, uint64_t limit, uint8_t type,
	               uint8_t dpl, uint8_t l, uint8_t db, uint8_t g)
{
	limit &= 0xfffff;
	type &= 0xf;

	sd->limit_low    = limit & 0xffff;
	sd->base_low     = base & 0xffff;
	sd->w2 = ((base >> 16) & 0xff) | ((uint64_t)type << 8) | ((uint64_t)1 << 12)
		| ((uint64_t)dpl << 13) | ((uint64_t)1 << 15);
	sd->w3 = ((limit >> 16) & 0xf) | ((uint64_t)l << 5) | ((uint64_t)db << 6)
		| ((uint64_t)g << 7) | (((base >> 24) & 0xff) << 8);
	return;
}


void set_gdt_tss_desc(struct gdt_tss_desc *e, uint64_t base, uint32_t limit,
                   uint8_t type, uint8_t dpl, uint8_t g)
{
    limit &= 0xfffff;
    type &= 0xf;

    e->limit_low = limit & 0xffff;
    e->base_low = base & 0xffff;
    e->w2 = ((base >> 16) & 0xff) | ((uint64_t)type << 8) | ((uint64_t)dpl << 13) | ((uint64_t)1 << 15); // 15th bit is Pflag
    e->w3 = ((limit >> 16) & 0xf) | ((uint64_t)g << 7) | (((base >> 24) & 0xff) << 8);
    e->base_high = base >> 32;
    e->reserved = 0;
}

void set_idt_gate(struct idt_gate *idt, uint64_t base, uint16_t selector, uint8_t flags)
{
	idt->offset_low = (uint16_t)(base & 0xffffULL);
	idt->selector = (uint16_t)selector;
	idt->ist = 0;
	idt->flags = flags;
	idt->offset_mid = (uint16_t)((base & 0xffff0000ULL) >> 16);
	idt->offset_hi = (uint16_t)((base & 0xffffffff00000000ULL) >> 32);
	idt->zero = 0;
	return;
}

void set_idt(uint8_t idtidx, void* handler)
{
	struct idt_gate *idt = (struct idt_gate *) ADR_IDT;
	set_idt_gate(idt + idtidx, (uint64_t) handler, 8, IDT_GATE_FLAG_PRESENT | IDT_GATE_FLAG_INTGATE);
}

void set_syscall(uint8_t idtidx, void* handler)
{
	struct idt_gate *idt = (struct idt_gate *) ADR_IDT;
	set_idt_gate(idt + idtidx, (uint64_t) handler, 8, IDT_GATE_FLAG_PRESENT | IDT_GATE_FLAG_INTGATE | IDT_GATE_FLAG_DPL_USER);
}
