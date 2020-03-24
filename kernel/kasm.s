section .text

[BITS 32]

global kentry
global _hlt

extern _kstart

FONT_ADR	EQU	0x0010_0000

kentry:
	; Save font addr
	mov 	esi, 0x7c00 + 512
	movzx	eax, word [esi + 0]
	movzx	ebx, word [esi + 2]
	shl	eax, 4
	add 	eax, ebx
	mov	[FONT_ADR], eax

	call 	_kstart

_hlt:
	hlt
	ret
