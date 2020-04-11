global _io_in8
global _io_out8
global _io_in16
global _io_out16
global _io_in32
global _io_out32
global _io_cli
global _io_sti
global _io_hlt
global _io_stihlt
global _asm_int_keyboard
global _asm_int_pit
global _asm_int_r8169
global _load_idtr
global _load_gdtr
global _load_tr
global _farjmp

extern _int_keyboard
extern _int_pit
extern _r8169_int_handler

; rdi - used to pass 1st argument to functions
; rsi - used to pass 2nd argument to functions
; rdx - used to pass 3rd argument to functions
; rcx - used to pass 4th argument to functions
; r8 - used to pass 5th argument to functions
; r9 - used to pass 6th argument to functions

[BITS 64]
_io_in8:	; uint8_t io_in8(uint16_t port)
	mov	dx, di
	mov	rax, 0
	in	al, dx
	ret

_io_out8:	; void io_out8(int port, int data)
	mov	dx, di
	mov	al, sil
	out	dx, al
	ret

_io_in16:	; uint16_t io_in16(uint16_t port)
	mov	dx, di
	mov	rax, 0
	in	ax, dx
	ret

_io_out16:	; void io_out8(uint16_t port, uint16_t data)
	mov	dx, di
	mov	ax, si
	out	dx, ax
	ret

_io_in32:	; uint32_t io_in32(uint16_t port)
	mov	dx, di
	mov	rax, 0
	in	eax, dx;
	ret

_io_out32:	; void io_out32(uint16_t port, uint32_t data)
	mov	dx, di;
	mov	eax, esi
	out	dx, eax
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
	push    rax
	push    rcx
	push    rdx
	push    rbx
	push    rbp
	push    rsi
	push    rdi
	push	r8
	push	r9
	push	r10
	push	r11

	call 	_int_keyboard

	pop	r11
	pop	r10
	pop	r9
	pop	r8
	pop     rdi
	pop     rsi
	pop     rbp
	pop     rbx
	pop     rdx
	pop     rcx
	pop     rax
	iretq

_asm_int_pit:
	push    rax
	push    rcx
	push    rdx
	push    rbx
	push    rbp
	push    rsi
	push    rdi
	push	r8
	push	r9
	push	r10
	push	r11

	call 	_int_pit

	pop	r11
	pop	r10
	pop	r9
	pop	r8
	pop     rdi
	pop     rsi
	pop     rbp
	pop     rbx
	pop     rdx
	pop     rcx
	pop     rax
	iretq


_asm_int_r8169:
	push    rax
	push    rcx
	push    rdx
	push    rbx
	push    rbp
	push    rsi
	push    rdi
	push	r8
	push	r9
	push	r10
	push	r11

	call 	_r8169_int_handler

	pop	r11
	pop	r10
	pop	r9
	pop	r8
	pop     rdi
	pop     rsi
	pop     rbp
	pop     rbx
	pop     rdx
	pop     rcx
	pop     rax
	iretq

; void load_idtr(void *idtr)
_load_idtr:
	lidt	[rdi]
	ret

; void load_gdtr(void *gdtr)
_load_gdtr:
	lgdt	[rdi]
	ret

; void load_tr(uint16_t tr);
_load_tr:
	ltr	di
	ret
;
;; taskswitch(int eip, int cs)
;_farjmp:
;	jmp	far [esp+4]
;	ret
