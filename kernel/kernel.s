%include	"../asmlib/defines.s"

ORG KERNEL_LOAD

; specify 32 bit code
[BITS 32]

kernel:
	; test
	mov	[0x000A_0000 + 80 + 5], byte 0xFF
	mov	[0x000A_0000 + 160 + 5], byte 0xFF
	mov	[0x000A_0000 + 320 + 5], byte 0xFF


; width 640, height 480
; We have 80 bytes in each row. 80 bytes can represents 640 (80 * 8) dots
; 0x000A_0000 + (640 / 8) * 0 
; 0x000A_0000 + (640 / 8) * 1
; ....
; ....
; 0x000A_0000 + (640 / 8) * 479


	; loop
.fin:
	jmp	$


