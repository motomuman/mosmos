global _io_in8
global _io_out8
global _io_in16
global _io_out16
global _io_in32
global _io_out32
global _io_cli
global _io_sti
global _io_hlt
global _io_stihlt
global _load_idtr
global _load_gdtr
global _load_tr
global _task_switch
global _get_rflags
global _set_rflags

extern _schedule

; rdi - used to pass 1st argument to functions
; rsi - used to pass 2nd argument to functions
; rdx - used to pass 3rd argument to functions
; rcx - used to pass 4th argument to functions
; r8 - used to pass 5th argument to functions
; r9 - used to pass 6th argument to functions

_io_in8:	; uint8_t io_in8(uint16_t port)
	mov	dx, di
	mov	rax, 0
	in	al, dx
	ret

_io_out8:	; void io_out8(int port, int data)
	mov	dx, di
	mov	al, sil
	out	dx, al
	ret

_io_in16:	; uint16_t io_in16(uint16_t port)
	mov	dx, di
	mov	rax, 0
	in	ax, dx
	ret

_io_out16:	; void io_out8(uint16_t port, uint16_t data)
	mov	dx, di
	mov	ax, si
	out	dx, ax
	ret

_io_in32:	; uint32_t io_in32(uint16_t port)
	mov	dx, di
	mov	rax, 0
	in	eax, dx;
	ret

_io_out32:	; void io_out32(uint16_t port, uint32_t data)
	mov	dx, di;
	mov	eax, esi
	out	dx, eax
	ret

_io_cli:
	cli
	ret

_io_sti:
	sti
	ret

_io_hlt:
	hlt
	ret

_io_stihlt:
	sti
	hlt
	ret

; void load_idtr(void *idtr)
_load_idtr:
	lidt	[rdi]
	ret

; void load_gdtr(void *gdtr)
_load_gdtr:
	lgdt	[rdi]
	ret

; void load_tr(uint16_t tr);
_load_tr:
	ltr	di
	ret

_test_and_set:
	lock bts [rdi], rsi ;cf test_and_set(*rdi, rsi)
	jnc	.success    ; if(0 == cf) -> success
.fail
	mov	rax, rsi
	ret
.success
	mov	rax, 0
	ret

_get_rflags:
	pushfq
	pop	rax
	ret

_set_rflags:
	push	rdi
	popfq
	ret

_task_switch:
	push	rbp
	push	rbx
	push	rcx
	push	rdx
	push	rdi
	push	rsi
	push	r8
	push	r9
	push	r10
	push	r11
	push	r12
	push	r13
	push	r14
	push	r15

	call 	_schedule 	; _schedule returns uint64_t rsp[2];
				; rsp[0] is pointer to current task rsp
				; rsp[1] is pointer to next task rsp

	mov	rdi, [rax]	; rdi = rsp[0] (*current_rsp)
	mov	rsi, [rax + 8]	; rsi = rsp[1] (*next_rsp)

	cmp	rdi, 0
	jz	.return_to_task_switch_caller

	; save stack frames
	mov	rdx,rsp
	mov	rax, 16
	push	rax	;ss
	push	rdx	;rsp
	pushfq		;rflags
	mov	rax, 8
	push	rax	;cs
	mov	rax, .return_to_task_switch_caller
	push	rax	;rip

	push	rax
	push	rbx
	push	rcx
	push	rdx
	push	r8
	push	r9
	push	r10
	push	r11
	push	r12
	push	r13
	push	r14
	push	r15
	push	rsi
	push	rdi
	push	rbp
	push	fs
	push	gs

	mov	rax, [rsi]	; rax = next_rsp
	mov	[rdi], rsp	; save current rsp
	mov	rsp, rax	; set next rsp

	; pop next task registers and jump to next task
	; possibly jump to .return_to_task_switch_caller of next task
	pop	gs
	pop	fs
	pop	rbp
	pop	rdi
	pop	rsi
	pop	r15
	pop	r14
	pop	r13
	pop	r12
	pop	r11
	pop	r10
	pop	r9
	pop	r8
	pop	rdx
	pop	rcx
	pop	rbx
	pop	rax
	iretq

.return_to_task_switch_caller:
	pop	r15
	pop	r14
	pop	r13
	pop	r12
	pop	r11
	pop	r10
	pop	r9
	pop	r8
	pop	rsi
	pop	rdi
	pop	rdx
	pop	rcx
	pop	rbx
	pop	rbp
	ret
