ORG 0x9000

bootmon:
	push	word .s0
	call	puts
	add	sp, 2

.s0	db "bootmon loaded", 0x0a, 0x0d, 0

%include	"../asmlib/puts.s"
times (1024*8) - ($ - $$) db 0	; 8K byte
