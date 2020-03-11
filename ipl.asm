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

	push	word msg
	call	puts
	add	sp, 2

	; loop
fin:
	hlt
	jmp	fin


puts:
	push	bp
	mov	bp, sp

	; save regs
	push 	ax
	push	bx
	push	si

	; -----------------
	;|      si        |   <- [sp]  
	; -----------------
	;|      bx        |
	; -----------------
	;|      ax        | 
	; -----------------
	;|   original bp  |   <- [bp]
	; -----------------
	;|   ret address  |
	; -----------------
	;|   msg address  |   <- [bp + 4]
	; -----------------

	mov	si, [bp+4]

	mov	ah, 0x0E
	mov	bx, 0x0000

	; clear direction flag (set df 0)
	cld

.putsloop:

	; load [ds:si] to al and
	; increment si if df=1
	; decrement si if df=0
	lodsb

	; if al == 0 goto putsend
	cmp	al, 0
	je	.putsend

	int	0x10
	jmp	.putsloop

.putsend:
	; restore regs
	pop	si
	pop	bx
	pop	ax

	mov	sp, bp
	pop	bp

	ret

; 0x21: !
; 0x0a: new line
; 0x0d: carriage return
msg:	db "Hello, MOSMOS", 0x21, 0x0a, 0x0d, 0
