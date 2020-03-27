global _io_in8
global _io_out8
global _io_cli
global _io_sti
global _io_hlt
global _io_stihlt
global _asm_int_keyboard
global _load_idtr
global _load_gdtr
global _load_tr
global _taskswitch3
global _taskswitch4

extern _int_keyboard

[BITS 32]
_io_in8:	; int io_in8(int port)
	mov	edx, [esp+4]
	mov	eax, 0
	in	al, dx;
	ret

_io_out8:	; void io_out8(int port, int data)
	mov	edx, [esp+4]
	mov	al, [esp + 8]
	out	dx, al
	ret

_io_cli:
	cli
	ret

_io_sti:
	sti
	ret

_io_hlt:
	hlt
	ret

_io_stihlt:
	sti
	hlt
	ret

_asm_int_keyboard:
	pusha
	push	ds
	push	es

	mov	ax, 0x0010
	mov	ds, ax
	mov	es, ax

	call 	_int_keyboard

	pop	es
	pop	ds
	popa

	iret

; void load_idtr(int limit, int addr)
; ESP + 4: limit
; ESP + 8: addr
;
; lidt loads data to idtr register (48bit)
; lower 16 bit of idtr (first 2byte): limit
; higher 32 bit of idtr (first 2byte): addr for IDTR data
_load_idtr:
	mov	ax,[esp+4]
	mov	[esp+6],ax
	lidt	[esp+6]
	ret

; void load_gdtr(int limit, int addr)
; ESP + 4: limit
; ESP + 8: addr
;
; lgdt loads data to gdtr register (48bit)
; lower 16 bit of idtr (first 2byte): limit
; higher 32 bit of idtr (first 2byte): addr for IDTR data
_load_gdtr:
	mov	ax,[esp+4]
	mov	[esp+6],ax
	lgdt	[esp+6]
	ret

; void load_tr(int tr);
_load_tr:
	ltr	[esp+4]
	ret

_taskswitch4:
	JMP	4*8:0
	ret

_taskswitch3:
	JMP	3*8:0
	ret
