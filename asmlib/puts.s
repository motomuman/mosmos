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
