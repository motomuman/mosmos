; read_chs(drive, sect, dst)
; param
; 	drive: address of drive struct
; 	sect: number of sect to read
; 	dst: buffer address
; ret
;	ax: number of success read sector

read_chs:
	
	push	bp
	mov	bp,sp

	push	3	; retry limit
	push	0	; read finished sector number

	; -----------------
	;|  done sec num  |   <- [sp]  
	; -----------------
	;|   retry limit  | 
	; -----------------
	;|   original bp  |   <- [bp]
	; -----------------
	;|   ret address  |
	; -----------------
	;|   driver info  |
	; -----------------
	;|    sect num    |
	; -----------------
	;|      dst       |
	; -----------------

	; save regs
	push	bx
	push	cx
	push	dx
	push	es
	push	si

	mov	si, [bp+4]

	; convert driver info for bios call
	mov	ch, [si + drive.cyln + 0]	;lower byte of cylnder num
	mov	cl, [si + drive.cyln + 1]	;higher byte of cylnder num
	shl	cl, 6				; cl<<=6
	or	cl,[si + drive.sect]		; cl |= sect

	;read sector
	mov	dh,[si + drive.head]
	mov	dl,[si]
	mov	ax,0x0000
	mov	es,ax
	mov	bx,[bp+8]		; dst

.retry:
	mov	ah,2			; Read
	mov	al,[bp+6]		; number of sect to read

	int	0x13			; bios call
					; return value
					; cf: error flag
					; ah: disk status code
					; al: number of successful read


	jnc	.11E			; if no carry (read success), go to 11E
	mov	al,0
	jmp	.10E

.11E:
	cmp	al,0			; if al == 0, set ZF=1
	jne	.10E			; if ZF == 0, go to 10E
	mov	ax,0

	; if still have retry count, retry
	dec	word [bp-2]
	jnz	.retry

.10E:
	; drop disk status code
	mov	ah,0

	; restore regs
	pop	si
	pop	es
	pop	dx
	pop	cx
	pop	bx

	mov	sp,bp
	pop	bp

	ret

