section .text

[BITS 64]

global kentry
global _FONT_ADR

extern _kstart

kentry:
	; Save font addr
	mov 	rsi, 0x7c00 + 512
	movzx	rax, word [rsi + 0]
	movzx	rbx, word [rsi + 2]
	shl	rax, 4
	add 	rax, rbx
	mov	[_FONT_ADR], rax

	call 	_kstart

_FONT_ADR: dd 0,0
