
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

; read_lba(drive, lba, sect, dst)
; param
; 	drive: addr for drive param struct
; 	lba: LBA
; 	sect: number of sect to read
; 	dst: buffer address
; ret
;	ax: number of success read sector

read_lba:
	push	bp
	mov	bp, sp

	; save regs
	push	si

	mov	si, [bp + 4]		;drive

	mov	ax, [bp + 6]		;LBA
	cdecl lba_chs, si, .chs, ax

	; copy drive number
	mov	al,[si + drive.no]
	mov	[.chs + drive.no], al

	cdecl	read_chs, .chs, word [bp + 8], word [bp + 10]

	pop	si

	mov	sp,bp
	pop	bp

	ret



ALIGN 2
.chs:	times drive_size	db	0


; lba_chs(drive, drv_chs, lba)
; param
; 	drive: addr for drive param struct
; 	drv_chs: addr for drive param struct to store after chs conversion
; 	lba: LBA
; ret
;	ax: 0 in case of failure
;
; LBA -> CHS calculation
; chs cylnder = LBA / (head num * sect num)
; chs track(head) = (LBA % (head num * sect num)) / sect_num
; chs sect = (LBA % (head num * sect num)) % sect_num + 1
lba_chs:
	push	bp
	mov	bp, sp

	; save regs
	push	ax
	push	bx
	push	cx
	push	es
	push	si
	push	di

	mov	si,[bp+4]		; drive
	mov	di,[bp+6]		; drv_chs


	mov	al,[si + drive.head]	; al = head num
	mul	byte [si + drive.sect]	; ax = max head * max sect
	mov	bx, ax			; bx = max head * max sect

	mov	dx, 0			; dx = LBA (higher 2 byte)
	mov	ax, [bp + 8]		; ax = LBA (lower 2 byte)
	div	bx			; DX = DX:AX % BX (LBA % (head num * sect num))
					; AX = DX:AX / BX (LBA / (head num * sect num))

	mov	[di+drive.cyln], ax	; store chs cyln

	mov	ax, dx			; ax = (LBA % (head num * sect num))
	div	byte [si + drive.sect]	; ah = (LBA % (head num * sect num)) % sect_num
					; ah = (LBA % (head num * sect num)) / sect_num

	movzx	dx, ah			; dx = sect
	inc	dx			; sect is 1-indexed

	mov	ah, 0x00		; ax = chs head

	mov	[di + drive.head], ax	; store chs head
	mov	[di + drive.sect], dx	; store chs sect

	; restore regs
	pop	di
	pop	si
	pop	es
	pop	cx
	pop	bx
	pop	ax

	mov	sp,bp
	pop	bp

	ret
