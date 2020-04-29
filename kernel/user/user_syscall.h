#ifndef _USER_SYSCALL_H_
#define _USER_SYSCALL_H_

#include <stdint.h>

int sys_print_str(char *str);
int sys_print_num(uint64_t num);
int sys_raw_socket(uint8_t proto);
int sys_raw_socket_send(int sock, uint32_t dip, uint8_t *buf, int size, int ttl);
int sys_raw_socket_recv(int sock, uint8_t *buf, int size);
int sys_udp_socket();
int sys_udp_socket_send(int sock, uint32_t dip, uint16_t dport, uint8_t *buf, int size);
int sys_udp_socket_recv(int sock, uint8_t *buf, int size);
int sys_key_getc(int is_blocking);
uint64_t sys_get_tick();
int sys_tcp_socket();
int sys_tcp_socket_connect(int sock, uint32_t dip, uint16_t dport);
int sys_tcp_socket_send(int sock, uint8_t *buf, int size);
int sys_tcp_socket_recv(int sock, uint8_t *buf, int size, int timeout_msec);
int sys_tcp_socket_close(int sock);
int sys_tcp_socket_bind(int sock, uint32_t sip, uint16_t sport);
int sys_tcp_socket_listen(int sock);
int sys_tcp_socket_accept(int sock);

#endif
