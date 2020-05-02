%include	"./defines.s"
%include	"./macro.s"

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
	jnz	.fail
	JMP	stage_get_drive_param

.fail:
	; Faled to load next stages
	cdecl puts, .load_error_msg
	jmp	$


.hello_msg:	db "ipl: Hello, MOSMOS", 0x21, 0x0a, 0x0d, 0
.load_error_msg:db "ipl: Failed to load next stages", 0x0a, 0x0d, 0

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
%include	"./puts.s"
%include	"./drive.s"

;------------------------------------
; Boot flag, end of first 512 bytes
;------------------------------------
times	510 - ($ - $$) db 0x00
db	0x55, 0xaa

;------------------------------------
; font data (0x7c00 + 512)
;------------------------------------
FONT:
.seg:	dw 0
.off:	dw 0

;------------------------------------
; stage: get drive parameters
;------------------------------------
stage_get_drive_param:
	cdecl 	puts, .enter_msg

	; Get drive parameter
	cdecl 	get_drive_param, BOOT 	; read_chs(drive, sect, dst)
					; return value
					; 	ax: 0 in case of failure
	cmp	ax, 0
	je	.fail
	jmp 	stage_get_bios_font

.fail:
	cdecl 	puts, .get_drive_param_fail_msg
	jmp	$

.enter_msg			db "stage_get_drive_param: Get drive params", 0x0a, 0x0d, 0
.get_drive_param_fail_msg      	db "stage_get_drive_param: Failed to get drive params", 0x0a, 0x0d, 0

;------------------------------------
; stage: get bios font data
;------------------------------------
stage_get_bios_font:
	cdecl 	puts, .enter_msg
	; Get bios font data
	mov	ax,0x1130
	mov	bh,0x06
	int	10h
	mov	[FONT+0],es
	mov	[FONT+2],bp
	jmp stage_load_kernel

.enter_msg	db "stage_get_bios_font: Get bios font", 0x0a, 0x0d, 0

;------------------------------------
; stage: enter a20 and load kernel
;------------------------------------
stage_load_kernel:
	cdecl 	puts, .enter_msg

	; Enable a20 gate
	call .enable_a20
	cdecl	puts, .enable_a20_success_msg

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
	je	.fail
	jmp	stage_video_mode

.fail:
	cdecl 	puts, .load_kernel_fail_msg
	jmp	$

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


.enter_msg		db "stage_load_kernel: Enable A20 gate and load kernel", 0x0a, 0x0d, 0
.enable_a20_success_msg	db "stage_load_kernel: Enabled A20 gate", 0x0a, 0x0d, 0
.load_kernel_fail_msg	db "stage_load_kernel: Failed to load kernel", 0x0a, 0x0d, 0

;------------------------------------
; stage: update video mode
;------------------------------------
stage_video_mode:
	; Wait key press so that I can see debug log before move to protect mode
	; cdecl puts, .ask_for_protect_mode_msg
	; mov	ah, 0x00
	; int	0x16

	; Update video mode
	mov	ax, 0x0012	; VGA 640x480
	int	0x10

	jmp stage_move_protect_mode

.ask_for_protect_mode_msg	db "Push Key to move protect mode...", 0x0a, 0x0d, 0


; check description in kernel/ksctbl.c
ALIGN 4, db 0
GDT:			dq	0x00_0_0_0_0_000000_0000	; NULL
.cs64:                  dq      0x00_A_F_9_A_000000_FFFF        ; CODE 4G (64bit)
.ds:			dq	0x00_C_F_9_2_000000_FFFF	; DATA 4G
.cs:			dq	0x00_C_F_9_A_000000_FFFF	; CODE 4G
.gdt_end:

SEL_CODE	equ	.cs - GDT	; segment reg value to use code segment
SEL_DATA	equ	.ds - GDT	; segment reg value to use data segment
SEL_CODE_64     equ     .cs64 - GDT     ; segment reg value to use code64 segment


GDTR:	dw	GDT.gdt_end - GDT - 1
	dd	GDT


IDTR:	dw	0
	dd	0

;----------------------------------------------
; stage: enter protect mode and jump to kernel
;----------------------------------------------
stage_move_protect_mode:
	; Move to protect mode
	cli
	lgdt	[GDTR]
	lidt	[IDTR]
	mov	eax,cr0
	or	ax, 1
	mov	cr0, eax

	jmp	$ + 2		; jump to discard prefetch

[BITS 32]
	db	0x66
	jmp	SEL_CODE:CODE_32

CODE_32:
	; Initialize selectors
	mov	ax, SEL_DATA
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax
	mov	ss, ax

	; Relocate kernel (will replace this by paging)
	mov	ecx, (KERNEL_SIZE) / 4
	mov	esi, BOOT_END
	mov	edi, KERNEL_LOAD
	cld
	rep 	movsd

;----------------------------------------------
; setup page table. Description is in the bottom of this file!!
;----------------------------------------------
PAGE_SETUP:
	; Enable Page Address Extension (PAE)
	mov     eax, 0x20
	mov     cr4, eax

	;Initialize page table by zero
	mov     ebx, PGTBL
	mov     edi, ebx
	mov     eax, 0
	mov     ecx, 512*8*6/4			; 512 entries * 8bytes (64bit) * 6table / 4bytes
	rep     stosd           		; write eax to [edi, edi + ecx]

	; level3
	mov	eax, PGTBL + 0x1000 + 0x007 	;PGTBL + 0x1000 is address of level2 table
						;0x007 means R/W
	mov	[ebx], eax

	; Level2
LEVEL2:
	mov     edi, PGTBL + 0x1000 		; entry address
	mov     eax, PGTBL + 0x2000 + 0x007 	; PGTBL + 0x2000 is start address of level1 table
						; 0x007 means R/W
	mov     ecx, 4				; Create 4 entries in level2
.loop:
	mov     [edi], eax
	add     edi, 8
	add     eax, 0x1000
	loop    .loop

LEVEL1:
	mov     edi, PGTBL + 0x2000		; PGTBL + 0x2000 is start address of level1 table
	mov     eax, 0x087			; (10000111) R/W user access, 2MB paging
						; This is for temporary, this should be root access (0x83)
	mov     ecx, 512*4			; 4tables
.loop:
	mov     [edi], eax
	add     edi, 8
	add     eax, 0x00200000
	loop    .loop

	; Set page table register
	mov     cr3, ebx

	; Enable long mode
	mov     ecx, 0xc0000080
	rdmsr
	bts     eax, 8
	wrmsr

	; Activate page translation and long mode
	mov     eax, 0x80000001
	mov     cr0, eax

	;jmp to long-mode

	;worked
	jmp SEL_CODE_64:KERNEL_LOAD

	;worked
	;db 0xea
	;dd CODE_64
	;dw SEL_CODE_64

	;did not work
	;push    SEL_CODE_64
	;push    KERNEL_LOAD
	;push    CODE_64
	;lret

	times 	BOOT_SIZE - ($ - $$)	db	0

; I used 2MB for each page. And used 2048 pages to map 4GB liner
; I use 1table(entry0) in level3, 1table(entry0-3) in level2, and 4table (entry0 ~ 511) in level6
; Table occupies 512 entries * 8bytes * 6table = 24KB
;
;          Level3       Level2           Leve1
;
;          (PGTBL) (PGTBL + 0x1000)    (PGTBL + 0x2000)
; CR3 ->   Entry0  --+-- Entry0 ------+-- Entry0
;          Entry1    +-- Entry1 --+   +-- Entry1
;            .       +     .      |   |     .
;            .       +     .      |   |     .
;          Entry511  +-- Entry511 |   +-- Entry511
;                                 |
;                                 |   (PGTBL + 0x3000)
;                                 +------ Entry0
;                                 +------ Entry1
;                                 |         .
;                                 |         .
;                                 +------ Entry511
