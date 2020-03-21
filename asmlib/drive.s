
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

; get_drive_param(drive)
; param
; 	drive: address of drive struct
; ret
;	ax: 0 in case of failure
get_drive_param:
	push	bp
	mov	bp, sp

	; save regs
	push	bx
	push	cx
	push	es
	push	si
	push	di

	mov	si,[bp + 4]	; get address of drive param struct

	mov	ax,0
	mov	es,ax
	mov	di,ax

	mov	ah,8			; ah=8: read drive parameter
	mov	dl,[si + drive.no]	; drive number

	int	0x13			; bios call to read drive parameters
					; return
					; CF: if error, 1
					; AH: disk status code
					; BL: drive type (only for floppy)
					; CH: lower 8 bit of max track number(10bit)
					; CL: higher 2 bit=higher 2 bit of max track number, lower 6 bit = max sector number
					; DH: max head number
					; DL: max drive number
					; ES:DI: pointer to floppy disk parameter table

	jc	.fail

	; get sector number
	mov	al,cl
	and	ax, 0x3f		; get lower 6 bit (max sector number)

	; get cylnder number
	shr	cl,6			; cl>>=6
					; cx
					; ch:lower 8 bit of max track number, cl: highwer 2 bit of max track number
	ror	cx,8			; rotate to right
	inc	cx			; cx++

	; get head number
	movzx	bx,dh
	inc	bx

	mov	[si + drive.cyln],cx
	mov	[si + drive.head],bx
	mov	[si + drive.sect],ax

	jmp	.success
.fail:
	mov	ax,0

.success:
	pop	di
	pop	si
	pop	es
	pop	cx
	pop	bx

	mov	sp,bp
	pop	bp

	ret
