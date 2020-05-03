#include "user_syscall.h"
#include "user_syscall_asm.h"

int sys_print_str(char *str)
{
	return sys_call(1, (uint64_t) str, 0, 0, 0, 0);
}

int sys_print_num(uint64_t num)
{
	return sys_call(2, num, 0, 0, 0, 0);
}

int sys_raw_socket(uint8_t proto)
{
	return sys_call(3, (uint64_t)proto, 0, 0, 0, 0);

}

int sys_raw_socket_send(int sock, uint32_t dip, uint8_t *buf, int size, int ttl)
{
	return sys_call(4, (uint64_t)sock, (uint64_t)dip, (uint64_t)buf, (uint64_t)size, (uint64_t)ttl);
}

int sys_raw_socket_recv(int sock, uint8_t *buf, int size)
{
	return sys_call(5, (uint64_t)sock, (uint64_t)buf, (uint64_t)size, 0, 0);
}

int sys_udp_socket()
{
	return sys_call(6, 0, 0, 0, 0, 0);

}

int sys_udp_socket_send(int sock, uint32_t dip, uint16_t dport, uint8_t *buf, int size)
{
	return sys_call(7, (uint64_t)sock, (uint64_t)dip, (uint64_t) dport, (uint64_t)buf, (uint64_t)size);
}

int sys_udp_socket_recv(int sock, uint8_t *buf, int size)
{
	return sys_call(8, (uint64_t)sock, (uint64_t)buf, (uint64_t)size, 0, 0);
}

int sys_key_getc(int is_blocking)
{
	return sys_call(9, (uint64_t)is_blocking, 0, 0, 0, 0);
}

uint64_t sys_get_tick()
{
	return sys_call(10, 0, 0, 0, 0, 0);
}

int sys_tcp_socket()
{
	return sys_call(11, 0, 0, 0, 0, 0);
}

int sys_tcp_socket_connect(int sock, uint32_t dip, uint16_t dport)
{
	return sys_call(12, (uint64_t) sock, (uint64_t) dip, (uint64_t) dport, 0, 0);
}

int sys_tcp_socket_send(int sock, uint8_t *buf, int size)
{
	return sys_call(13, (uint64_t) sock, (uint64_t) buf, (uint64_t) size, 0, 0);
}

int sys_tcp_socket_recv(int sock, uint8_t *buf, int size, int timeout_msec)
{
	return sys_call(14, (uint64_t) sock, (uint64_t) buf, (uint64_t) size, (uint64_t) timeout_msec, 0);
}

int sys_tcp_socket_close(int sock)
{
	return sys_call(15, (uint64_t) sock, 0, 0, 0, 0);
}

int sys_tcp_socket_bind(int sock, uint32_t sip, uint16_t sport)
{
	return sys_call(16, (uint64_t) sock, (uint64_t) sip, (uint64_t) sport, 0, 0);
}

int sys_tcp_socket_listen(int sock)
{
	return sys_call(17, (uint64_t) sock, 0, 0, 0, 0);
}

int sys_tcp_socket_accept(int sock)
{
	return sys_call(18, (uint64_t) sock, 0, 0, 0, 0);
}
