ORG 0x7c00

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

	mov	[drive], dl		; save boot drive

	push	word msg
	call	puts
	add	sp, 2

	; load next 512 bytes
	mov	ah, 0x02		; Read
	mov	al, 10			; Number of sector to read
	mov	cx, 0x0002		; Selinder number
	mov	dh, 0x00		; Head number
	mov	dl, [drive]		; drive number
	mov	bx, 0x9000		; ES:BX = buffer address
	int	0x13			; call read
	jnc	.next

	; Faled to read sector
	push	word readerr
	call	puts
	add	sp, 2
	jmp 	.fin


.next
	; Get bios font data
	mov	ax,0x1130
	mov	bh,0x06
	int	10h
	mov	[FONT+0],es
	mov	[FONT+2],bp


	; Jump to bootmon
	JMP	0x9000


	; loop
.fin:
	jmp	$


; 0x21: !
; 0x0a: new line
; 0x0d: carriage return
msg:	db "Hello, MOSMOS", 0x21, 0x0a, 0x0d, 0

readerr: db "Read sector error", 0x0a, 0x0d, 0

; drive number
drive:	dw 0

%include	"../asmlib/puts.s"

; Boot flag, end of first 512 bytes
times	510 - ($ - $$) db 0x00
db	0x55, 0xaa

; font data (0x7c00 + 512)
FONT:
.seg:	dw 0
.off:	dw 0

