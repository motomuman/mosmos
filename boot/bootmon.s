ORG 0x9000

bootmon:
	push	word s0
	call	puts
	add	sp, 2


	call enable_a20

	push	word s1
	call	puts
	add	sp, 2

;-------------------
; Enable A20 gate
;-------------------
enable_a20:
	cli

	; write command to key board
	call	waitkbdout
	mov	al,0xd1
	out 	0x64,al

	; write data to key board
	call    waitkbdout
	mov	al,0xdf
	out 	0x60,al

	sti

	ret


; wait until writable
waitkbdout:
	in	al, 0x64
	test	al, 0x02
	jnz	waitkbdout ; if not writable, loop
	ret


s0	db "bootmon started", 0x0a, 0x0d, 0
s1	db "A20 gate enabled", 0x0a, 0x0d, 0
s2 	db "checking writeable", 0x0a, 0x0d, 0


%include	"../asmlib/puts.s"
