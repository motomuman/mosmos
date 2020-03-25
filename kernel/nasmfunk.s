global _io_in8
global _io_out8
global _hlt

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

_hlt:
	hlt
	ret


