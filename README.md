# mosmos

mosmos image
mbr:				0 		- 	0x0200(512)
bootmon:			0x200(512) 	- 	0x2000(8192)
kernel:				0x2000(8192)	-	0x4000(16384)

memory map
mbr + boot: 			0x00007c00 - 0x00009c00
kernel(initial):		0x00009c00 - 0x0000bc00
page table for long mode	0x00079000 - 0x00079fff
....
gdt table:			0x00090000 - 0x0009ffff
intrupption vec: 		0x00100000 - 0x001007ff
kernel(relocated): 		0x00101000 - 0x00103000
free(managed by mem lib): 	0x00a00000 -
