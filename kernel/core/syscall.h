#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#define SYSCALL_PRINT_STR 1
#define SYSCALL_PRINT_NUM 2
#define SYSCALL_RAW_SOCK 3
#define SYSCALL_RAW_SOCK_SEND 4
#define SYSCALL_RAW_SOCK_RECV 5
#define SYSCALL_UDP_SOCK 6
#define SYSCALL_UDP_SOCK_SEND 7
#define SYSCALL_UDP_SOCK_RECV 8

void init_syscall();

#endif
