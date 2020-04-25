#include <stdint.h>
#include "dsctbl.h"
#include "print.h"
#include "intasm.h"
#include "raw.h"
#include "syscall.h"
#include "lib.h"
#include "udp.h"
#include "keyboard.h"
#include "timer.h"

int syscall_print_str(uint64_t _str) {
	char *str = (char *) _str;
	printstr_app(str);
	return 0;
}

int syscall_print_num(uint64_t num) {
	printnum_app(num);
	return 0;
}

int syscall_raw_socket(uint64_t proto) {
	return raw_socket(proto);
}

int syscall_raw_socket_send(uint64_t sock, uint64_t dip, uint64_t _buf, uint64_t size, uint64_t ttl) {
	uint8_t *buf = (uint8_t *)_buf;
	raw_socket_send(sock, dip, buf, size, ttl);
	return 0;
}

int syscall_raw_socket_recv(uint64_t sock, uint64_t _buf, uint64_t size) {
	uint8_t *buf = (uint8_t *)_buf;
	return raw_socket_recv(sock, buf, size);
}

int syscall_udp_socket() {
	return udp_socket();
}

int syscall_udp_socket_send(uint64_t sock, uint64_t dip, uint64_t dport, uint64_t _buf, uint64_t size) {
	uint8_t *buf = (uint8_t *)_buf;
	udp_socket_send(sock, dip, dport, buf, size);
	return 0;
}

int syscall_udp_socket_recv(uint64_t sock, uint64_t _buf, uint64_t size) {
	uint8_t *buf = (uint8_t *)_buf;
	return udp_socket_recv(sock, buf, size);
}

int syscall_key_getc() {
	return key_getc();
}

int syscall_get_tick() {
	return get_tick();
}

int syscall_handler(uint64_t rdi, uint64_t rsi,
		uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9)
{

	switch(rdi) {
		case SYSCALL_PRINT_STR:
			return syscall_print_str(rsi);
		case SYSCALL_PRINT_NUM:
			return syscall_print_num(rsi);
		case SYSCALL_RAW_SOCK:
			return syscall_raw_socket(rsi);
		case SYSCALL_RAW_SOCK_SEND:
			return syscall_raw_socket_send(rsi, rdx, rcx, r8, r9);
		case SYSCALL_RAW_SOCK_RECV:
			return syscall_raw_socket_recv(rsi, rdx, rcx);
		case SYSCALL_UDP_SOCK:
			return syscall_udp_socket();
		case SYSCALL_UDP_SOCK_SEND:
			return syscall_udp_socket_send(rsi, rdx, rcx, r8, r9);
		case SYSCALL_UDP_SOCK_RECV:
			return syscall_udp_socket_recv(rsi, rdx, rcx);
		case SYSCALL_KEY_GETC:
			return syscall_key_getc();
		case SYSCALL_GET_TICK:
			return syscall_get_tick();
		default:
			printstr_log("unknown syscall rdi:");
			printnum_log(rdi);
			printstr_app("\n");
			panic();
	}
	return -1;
}

void init_syscall()
{
	set_syscall(0x80, asm_syscall_handler);
}
