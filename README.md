# mosmos

mosmos image
mbr:				0 		- 	0x01ff(512)
bootmon:			0x200(512) 	- 	0x1ffff(8192)
kernel:				0x2000(8192)	-	0x4000(16384)

memory map
mbr + boot: 			0x00007c00 - 0x00009bff (8KB)
kernel(initial):		0x00009c00 - 0x00009fff (32KB)
page table for long mode	0x00079000 - 0x00079fff (4KB)
....
gdt table:			0x00090000 - 0x00090fff (4KB)
intrupption vec: 		0x00091000 - 0x00092fff (8KB)
kernel(relocated): 		0x00101000 - 0x00109000 (32KB)
kstack for userapp(temporary)              - 0x00900000
free(managed by mem lib): 	0x00a00000 -
