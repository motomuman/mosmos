BOOT_LOAD	equ	0x7c00

BOOT_SIZE	equ	(1024 * 8)
; size of each sector
SECT_SIZE	equ	512
; number of sector for boot asm
BOOT_SECT	equ	(BOOT_SIZE/SECT_SIZE)

BOOT_END	equ	(BOOT_LOAD + BOOT_SIZE)

KERNEL_LOAD	equ	0x0010_1000
KERNEL_SIZE	equ	(1024 * 8)
KERNEL_SECT	equ	(KERNEL_SIZE/SECT_SIZE)

struc drive
	.no	resw	1
	.cyln	resw	1
	.head	resw	1
	.sect	resw	1
endstruc
