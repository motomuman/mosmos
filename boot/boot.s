%include	"../asmlib/defines.s"
%include	"../asmlib/macro.s"

ORG BOOT_LOAD

entry:
	; BIOS Parameter Block
	jmp ipl
	times 90 - ($ - $$) db 0x90

	; Initial Program Loader
ipl:
	cli				; Disable int

	;Reset segment register
	mov	ax, 0x0000
	mov	ds, ax
	mov	es, ax
	mov	ss, ax

	; Setup stack	
	mov	sp, 0x7c00

	sti				; Enable int

	mov	[BOOT + drive.no], dl	; save boot drive

	cdecl puts, .hello_msg

	; Read remaining boot loader
	mov	bx, BOOT_SECT - 1		; remaining boot sector
	mov	cx, BOOT_LOAD + SECT_SIZE	; destination

	cdecl	read_chs, BOOT, bx, cx		; read_chs(drive, sect, dst)
						; ax: number of sect successfly read

	cmp	ax, bx				; if ax == bx, set ZF=1
	jz	.read_success

	; Faled to read sector
	cdecl puts, .read_error_msg
	jmp 	.fin

.read_success:
	; Jump to bootmon
	cdecl puts, .read_success_msg
	JMP	bootmon


	; loop
.fin:
	jmp	$


.hello_msg:	db "Hello, MOSMOS", 0x21, 0x0a, 0x0d, 0
.read_error_msg: db "Load error", 0x0a, 0x0d, 0
.read_success_msg: db "Load success", 0x0a, 0x0d, 0

ALIGN	2, db 0
BOOT: ; boot drive info
istruc drive
	at drive.no,	dw 0
 	at drive.cyln,	dw 0
  	at drive.head,	dw 0
  	at drive.sect,	dw 2
iend

;------------------------------------
; Libraries
;------------------------------------
%include	"../asmlib/puts.s"
%include	"../asmlib/drive.s"

;------------------------------------
; Boot flag, end of first 512 bytes
;------------------------------------
times	510 - ($ - $$) db 0x00
db	0x55, 0xaa


; font data (0x7c00 + 512)
FONT:
.seg:	dw 0
.off:	dw 0

bootmon:
	cdecl 	puts, .enter_msg

	; Get drive parameter
	cdecl 	get_drive_param, BOOT 	; read_chs(drive, sect, dst)
					; return value
					; 	ax: 0 in case of failure
	cmp	ax, 0
	je	.get_drive_param_fail

	; Get bios font data
	mov	ax,0x1130
	mov	bh,0x06
	int	10h
	mov	[FONT+0],es
	mov	[FONT+2],bp

	; Enable a20 gate
	call .enable_a20
	cdecl	puts, .enable_a20_finish

	; load kernel
	cdecl read_lba, BOOT, BOOT_SECT, KERNEL_SECT, BOOT_END 	; read_lba(drive, lba, sect, dst)
								; param
								; 	drive: addr for drive param struct
								; 	lba: LBA
								; 	sect: number of sect to read
								; 	dst: buffer address
								; ret
								;	ax: number of success read sector

	cmp	ax, 0
	je	.load_kernel_fail

	; temporarily jump to kernel before relocation/32bit
	jmp	BOOT_END

	jmp	.fin

.get_drive_param_fail:
	cdecl 	puts, .get_drive_param_fail_msg
	jmp	.fin


.load_kernel_fail:
	cdecl 	puts, .load_kernel_fail_msg
	jmp	.fin

	; loop
.fin:
	jmp	$

;-------------------
; Enable A20 gate
;-------------------
.enable_a20:

	push	bp
	mov	bp, sp

	; save regs
	push 	ax

	cli

	; write command to key board
	call	.waitkbdout
	mov	al,0xd1
	out 	0x64,al

	; write data to key board
	call    .waitkbdout
	mov	al,0xdf
	out 	0x60,al

	sti

	; restore regs
	pop	ax

	mov	sp, bp
	pop	bp

	ret


; wait until writable
.waitkbdout:
	in	al, 0x64
	test	al, 0x02
	jnz	.waitkbdout ; if not writable, loop
	ret

.enter_msg	db "bootmon started", 0x0a, 0x0d, 0
.enable_a20_finish		db "A20 gate enabled", 0x0a, 0x0d, 0
.get_drive_param_fail_msg	db "Failed to get drive param", 0x0a, 0x0d, 0
.load_kernel_fail_msg		db "Failed to load kernel", 0x0a, 0x0d, 0
