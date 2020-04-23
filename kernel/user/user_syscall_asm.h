#ifndef _USER_SYSCALL_ASM_H_
#define _USER_SYSCALL_ASM_H_

int sys_call(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9);

#endif
