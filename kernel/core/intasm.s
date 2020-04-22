global _asm_int_keyboard
global _asm_int_pit
global _asm_int_r8169
global _asm_syscall_handler
global _asm_int_0
global _asm_int_1
global _asm_int_2
global _asm_int_3
global _asm_int_4
global _asm_int_5
global _asm_int_6
global _asm_int_7
global _asm_int_8
global _asm_int_9
global _asm_int_10
global _asm_int_11
global _asm_int_12
global _asm_int_13
global _asm_int_14
global _asm_int_15
global _asm_int_16
global _asm_int_17
global _asm_int_18
global _asm_int_19

extern _int_keyboard
extern _int_pit
extern _r8169_int_handler
extern _syscall_handler
extern _int_default

_asm_int_keyboard:
	push    rax
	push    rcx
	push    rdx
	push    rbx
	push    rbp
	push    rsi
	push    rdi
	push	r8
	push	r9
	push	r10
	push	r11

	call 	_int_keyboard

	pop	r11
	pop	r10
	pop	r9
	pop	r8
	pop     rdi
	pop     rsi
	pop     rbp
	pop     rbx
	pop     rdx
	pop     rcx
	pop     rax
	iretq

_asm_int_pit:
	push    rax
	push    rcx
	push    rdx
	push    rbx
	push    rbp
	push    rsi
	push    rdi
	push	r8
	push	r9
	push	r10
	push	r11

	call 	_int_pit

	pop	r11
	pop	r10
	pop	r9
	pop	r8
	pop     rdi
	pop     rsi
	pop     rbp
	pop     rbx
	pop     rdx
	pop     rcx
	pop     rax
	iretq

_asm_int_r8169:
	push    rax
	push    rcx
	push    rdx
	push    rbx
	push    rbp
	push    rsi
	push    rdi
	push	r8
	push	r9
	push	r10
	push	r11

	call 	_r8169_int_handler

	pop	r11
	pop	r10
	pop	r9
	pop	r8
	pop     rdi
	pop     rsi
	pop     rbp
	pop     rbx
	pop     rdx
	pop     rcx
	pop     rax
	iretq

_asm_syscall_handler:
	push    rcx
	push    rdx
	push    rbx
	push    rbp
	push    rsi
	push    rdi
	push	r8
	push	r9
	push	r10
	push	r11

	call 	_syscall_handler

	pop	r11
	pop	r10
	pop	r9
	pop	r8
	pop     rdi
	pop     rsi
	pop     rbp
	pop     rbx
	pop     rdx
	pop     rcx
	iretq

_asm_int_0:
	mov	rdi, 0
	call 	_int_default

_asm_int_1:
	mov	rdi, 1
	call 	_int_default

_asm_int_2:
	mov	rdi, 2
	call 	_int_default

_asm_int_3:
	mov	rdi, 3
	call 	_int_default

_asm_int_4:
	mov	rdi, 4
	call 	_int_default

_asm_int_5:
	mov	rdi, 5
	call 	_int_default

_asm_int_6:
	mov	rdi, 6
	call 	_int_default

_asm_int_7:
	mov	rdi, 7
	call 	_int_default

_asm_int_8:
	mov	rdi, 8
	call 	_int_default

_asm_int_9:
	mov	rdi, 9
	call 	_int_default

_asm_int_10:
	mov	rdi, 10
	call 	_int_default

_asm_int_11:
	mov	rdi, 11
	call 	_int_default

_asm_int_12:
	mov	rdi, 12
	call 	_int_default

_asm_int_13:
	mov	rdi, 13
	call 	_int_default

_asm_int_14:
	mov	rdi, 14
	call 	_int_default

_asm_int_15:
	mov	rdi, 15
	call 	_int_default

_asm_int_16:
	mov	rdi, 16
	call 	_int_default

_asm_int_17:
	mov	rdi, 17
	call 	_int_default

_asm_int_18:
	mov	rdi, 18
	call 	_int_default

_asm_int_19:
	mov	rdi, 19
	call 	_int_default
