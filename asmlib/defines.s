BOOT_LOAD	equ	0x7c00

BOOT_SIZE	equ	(1024 * 8)
; size of each sector
SECT_SIZE	equ	512
; number of sector for boot asm
BOOT_SECT	equ	(BOOT_SIZE/SECT_SIZE)


; mosmos image
; mbr:		0 	- 	512
; bootmon:	513 	- 	8192 (1024 * 8)
; kernel:	8193	-

; memory map
; 0x7c00 -


struc drive
	.no	resw	1
	.cyln	resw	1
	.head	resw	1
	.sect	resw	1
endstruc
