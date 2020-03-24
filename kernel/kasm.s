section .text

[BITS 32]

global kentry
global _write
global _hlt

extern _kstart

kentry:
	call 	_kstart


_write:
	mov     [0x000A_0f00 + 80 + 5], byte 0xFF
	mov     [0x000A_0f00 + 160 + 5], byte 0xFF
	mov     [0x000A_0f00 + 240 + 5], byte 0xFF
	mov     [0x000A_0f00 + 320 + 5], byte 0xFF
	ret

_hlt:
	hlt
	ret
