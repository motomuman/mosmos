
global _sys_print
; sys_print(char *);
_sys_print:
	push    rcx
	push    rdx
	push    rbx
	push    rbp
	push    rsi
	push    rdi
	push    r8
	push    r9
	push    r10
	push    r11

	; rdi - used to pass 1st argument to functions
	; rsi - used to pass 2nd argument to functions
	mov     rsi, rdi
	mov     rdi, 0x01
	int     0x80

	pop     r11
	pop     r10
	pop     r9
	pop     r8
	pop     rdi
	pop     rsi
	pop     rbp
	pop     rbx
	pop     rdx
	pop     rcx
	ret

