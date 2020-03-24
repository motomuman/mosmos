section .text

[BITS 32]

global _kentry
extern _kstart

_kentry:
	cli
	call	_kstart

	jmp	$
