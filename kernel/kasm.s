section .text

[BITS 32]

global kentry
global _FONT_ADR

extern _kstart
extern _int_keyboard

%macro outp 2
	mov	al, %2
	out	%1, al
%endmacro


kentry:
	; Save font addr
	mov 	esi, 0x7c00 + 512
	movzx	eax, word [esi + 0]
	movzx	ebx, word [esi + 2]
	shl	eax, 4
	add 	eax, ebx
	mov	[_FONT_ADR], eax

	call 	init_int
	call	init_pic
	outp	0x21, 0b_1111_1101	; enable keyboard interruption

	sti

	call 	_kstart


VECT_BASE	equ	0x0010_0000	;0010_0000:0010_07FF

; interrupt vector
ALIGN 4
IDTR:	dw	8*256 - 1
	dd	VECT_BASE

init_int:
	;save regs
	push 	eax
	push	ebx
	push	ecx
	push	edi

	; Interrupt descriptor
	;63                  |47     45          40       32|                  |15          0
	; ----------------------------------------------------------------------------------+
	; |    offset (H)    | P | DPL | DT | Type |        | Segment selector | offset (L) |
	; ----------------------------------------------------------------------------------+
	; int_handler[31:16] |      8       |   E  |  0 | 0 | 0 | 0 | 0 |  8   | int_handler[15:0]

	lea	eax, [int_keyboard]	; EAX = addr of int_keyboard
	mov	ebx, 0x008_8E00		; ebx = segment selector
					; 0008 <- kernel segment (first segment)
	xchg	ax, bx			; exchange lower word

	mov	[VECT_BASE + 8 * 0x21 + 0], ebx		; lower interrupt vectors
	mov	[VECT_BASE + 8 * 0x21 + 4], eax		; higher interrupt vectors
							; key board vector 0x21

	lidt	[IDTR]

	;restore regs
	pop	edi
	pop	ecx
	pop	ebx
	pop	eax

	ret

init_pic:
	push	eax

	; master config
	outp	0x20, 0x11
	outp	0x21, 0x20
	outp	0x21, 0x04
	outp	0x21, 0x05
	outp	0x21, 0xFF

	; slave config
	outp	0xa0, 0x11
	outp	0xa1, 0x28
	outp	0xa1, 0x02
	outp	0xa1, 0x01
	outp	0xa1, 0xFF

	pop	eax

	ret

int_keyboard:
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

_FONT_ADR: dd 0
