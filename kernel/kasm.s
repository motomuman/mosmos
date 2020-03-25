section .text

[BITS 32]

global kentry
global _hlt
global _FONT_ADR

extern _kstart

kentry:
	; Save font addr
	mov 	esi, 0x7c00 + 512
	movzx	eax, word [esi + 0]
	movzx	ebx, word [esi + 2]
	shl	eax, 4
	add 	eax, ebx
	mov	[_FONT_ADR], eax

	call 	_kstart

_hlt:
	hlt
	ret

_FONT_ADR: dd 0
