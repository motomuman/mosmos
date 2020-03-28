global _io_in8
global _io_out8
global _io_cli
global _io_sti
global _io_hlt
global _io_stihlt
global _io_load_eflags
global _io_store_eflags
global _asm_int_keyboard
global _asm_int_pit
global _load_idtr
global _load_gdtr
global _load_tr
global _load_cr0
global _store_cr0
global _taskswitch3
global _taskswitch4
global _memtest_sub

extern _int_keyboard
extern _int_pit

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

_asm_int_pit:
	pusha
	push	ds
	push	es

	mov	ax, 0x0010
	mov	ds, ax
	mov	es, ax

	call 	_int_pit

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

_memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
	;save regs
	push	edi
	push	esi
	push	ebx
	mov	esi, 0xaa55aa55
	mov	edi, 0x55aa55aa
	mov	eax, [esp + 12 + 4] ; 12(edi,esi,ebx) + 4
.loop:
	mov	ebx, eax
	add	ebx, 0xffc
	mov	edx, [ebx] ; edx = *p (save old value)
	mov	[ebx], esi
	xor 	dword [ebx], 0xffffffff
	cmp	edi, [ebx]
	jne	.fin
	xor	dword [ebx], 0xffffffff
	cmp	esi, [ebx]
	jne	.fin
	mov	[ebx], edx ; *p = edx (restore old value)
	add	eax, 0x10000
	cmp	eax, [esp + 12 + 8]
	jbe	.loop
	jmp	.allpass
.fin
	mov	[ebx], edx  ; *p = edx (restore old value)
.allpass
	pop	ebx
	pop	esi
	pop	edi
	ret

_load_cr0:
	mov	eax, cr0
	ret

_store_cr0:
	mov	eax,[esp+4]
	mov	cr0, eax
	ret

_io_store_eflags:
	mov	eax,[esp+4]
	push	eax
	popfd
	ret

_io_load_eflags:
	pushfd
	pop	eax
	ret
