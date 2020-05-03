#include <stdint.h>
#include "user_syscall.h"

#include "netutil.h"
#include "ip.h"
#include "print.h"
#include "icmp.h"
#include "dns.h"
#include "lib.h"

void exec_command(char *buf);

void userland_main() {
	int buf_pos;
	while(1) {
		sys_print_str("> ");
		buf_pos = 0;
		char buf[100];
		memset(buf, 0, 100);

		while(1) {
			char tmp_buf[2];
			char ch = sys_key_getc(1);
			if(ch == 0) {
				continue;
			}

			tmp_buf[0] = ch;
			tmp_buf[1] = 0;
			sys_print_str(tmp_buf);
			if(ch == 0x0a) {
				break;
			}
			buf[buf_pos] = ch;
			buf_pos++;
			if(buf_pos > 90) {
				sys_print_str("\ncommand too long\n");
				break;
			}
		}
		if(buf_pos > 0) {
			exec_command(buf);
		}
	}
}

uint32_t stoi(char *buf) {
	uint32_t buflen = strlen(buf);
	int i;
	uint32_t ret = 0;

	for(i = 0; i < buflen; i++) {
		if(buf[i] < '0' || buf[i] > '9') {
			//sys_print_str("ERROR: stoi can not handle non number\n");
			return 0;
		}
		ret *= 10;
		ret += buf[i] - '0';
	}
	return ret;
}

uint32_t parse_ipaddr(char *buf)
{
	int start = 0;
	int end = 0;
	uint32_t buf_len = strlen(buf);
	char tmpbuf[100];
	int dotcount = 0;
	uint32_t ret = 0;

	while(end < buf_len) {
		end++;
		if(buf[end] == '.') {
			dotcount++;
			//set block
			memset(tmpbuf, 0, 100);
			memcpy(tmpbuf, buf + start, end - start);
			ret <<= 8;
			ret += stoi(tmpbuf);
			start = end + 1;
			end = end + 1;
		}
	}

	if(dotcount != 3) {
		return 0;
	}

	// last block
	memset(tmpbuf, 0, 100);
	memcpy(tmpbuf, buf + start, end - start);
	ret <<= 8;
	ret += stoi(tmpbuf);
	return ret;
}

void exec_ping(char *buf)
{
	uint32_t ipaddr = parse_ipaddr(buf);
	int raw_sock = sys_raw_socket(IP_HDR_PROTO_ICMP);
	int udp_sock = sys_udp_socket();
	int seq = 0;

	if(ipaddr == 0) {
		ipaddr = resolve_addr(udp_sock, buf);
	}

	if(ipaddr == 0) {
		return;
	}

	sys_print_str("PING ");
	sys_print_str(buf);
	sys_print_str(" (");
	sys_print_num((ipaddr >> 24) &0xff);
	sys_print_str(".");
	sys_print_num((ipaddr >> 16) &0xff);
	sys_print_str(".");
	sys_print_num((ipaddr >> 8) &0xff);
	sys_print_str(".");
	sys_print_num((ipaddr >> 0) &0xff);
	sys_print_str(")\n");

	for(seq = 0; seq < 5; seq++) {
		uint8_t _buf[500];
		uint8_t *buf = _buf;

		struct icmp_hdr *icmphdr = (struct icmp_hdr *) buf;
		buf += sizeof(struct icmp_hdr);
		icmphdr->type = ICMP_HDR_TYPE_ECHO_REQUEST;
		icmphdr->code = 0;
		icmphdr->checksum = 0;
		icmphdr->echo.id = seq;
		icmphdr->echo.seqnum = seq;

		*(uint64_t *) buf = sys_get_tick();

		buf -= sizeof(struct icmp_hdr);

		//set checksum
		icmphdr->checksum = checksum(buf, sizeof(struct icmp_hdr) + 8);

		sys_raw_socket_send(udp_sock, ipaddr, buf, sizeof(struct icmp_hdr) + 8, 64);

		int ret = sys_raw_socket_recv(raw_sock, buf, sizeof(struct icmp_hdr) + sizeof(struct ip_hdr) + 8);

		if(ret == -1) {
			sys_print_str("timeout!\n");
			continue;
		}
		struct ip_hdr *iphdr = (struct ip_hdr *)buf;
		buf += sizeof(struct ip_hdr) + sizeof(struct icmp_hdr);

		uint64_t send_tick = *(uint64_t *) buf;
		uint64_t recv_tick = sys_get_tick();

		sys_print_str("seq=");
		sys_print_num(seq);
		sys_print_str(" ttl=");
		sys_print_num(iphdr->ttl);
		sys_print_str(" time=");
		sys_print_num(recv_tick - send_tick);
		sys_print_str(" ms\n");
	}
}

void exec_traceroute(char *buf)
{
	uint32_t ipaddr = parse_ipaddr(buf);
	int raw_sock = sys_raw_socket(IP_HDR_PROTO_ICMP);
	int udp_sock = sys_udp_socket();
	int ttl;

	if(ipaddr == 0) {
		ipaddr = resolve_addr(udp_sock, buf);
	}

	if(ipaddr == 0) {
		return;
	}

	sys_print_str("traceroute to ");
	sys_print_str(buf);
	sys_print_str(" (");
	sys_print_num((ipaddr >> 24) &0xff);
	sys_print_str(".");
	sys_print_num((ipaddr >> 16) &0xff);
	sys_print_str(".");
	sys_print_num((ipaddr >> 8) &0xff);
	sys_print_str(".");
	sys_print_num((ipaddr >> 0) &0xff);
	sys_print_str(")\n");

	for(ttl = 1; ttl < 20; ttl++) {
		uint8_t _buf[500];
		uint8_t *buf = _buf;

		struct icmp_hdr *icmphdr = (struct icmp_hdr *) buf;
		buf += sizeof(struct icmp_hdr);
		icmphdr->type = ICMP_HDR_TYPE_ECHO_REQUEST;
		icmphdr->code = 0;
		icmphdr->checksum = 0;
		icmphdr->echo.id = ttl;
		icmphdr->echo.seqnum = ttl;

		uint64_t send_tick = sys_get_tick();

		buf -= sizeof(struct icmp_hdr);

		//set checksum
		icmphdr->checksum = checksum(buf, sizeof(struct icmp_hdr));

		sys_raw_socket_send(udp_sock, ipaddr, buf, sizeof(struct icmp_hdr), ttl);

		int ret = sys_raw_socket_recv(raw_sock, buf, sizeof(struct icmp_hdr) + sizeof(struct ip_hdr));

		if(ret == -1) {
			sys_print_num(ttl);
			sys_print_str(" timeout!\n");
			continue;
		}
		struct ip_hdr *iphdr = (struct ip_hdr *)buf;
		buf += sizeof(struct ip_hdr);
		icmphdr = (struct icmp_hdr *)buf;
		buf += sizeof(struct icmp_hdr);

		uint64_t recv_tick = sys_get_tick();

		char domainbuf[200];
		memset(domainbuf, 0, 200);
		ret = resolve_host(udp_sock, ntoh32(iphdr->sip) , domainbuf, 200);

		sys_print_num(ttl);
		sys_print_str(" ");
		if(ret != -1){
			sys_print_str(domainbuf);
			sys_print_str("(");
			sys_print_num((ntoh32(iphdr->sip) >> 24) &0xff);
			sys_print_str(".");
			sys_print_num((ntoh32(iphdr->sip) >> 16) &0xff);
			sys_print_str(".");
			sys_print_num((ntoh32(iphdr->sip) >> 8) &0xff);
			sys_print_str(".");
			sys_print_num((ntoh32(iphdr->sip)) &0xff);
			sys_print_str(") ");
		} else {
			sys_print_num((ntoh32(iphdr->sip) >> 24) &0xff);
			sys_print_str(".");
			sys_print_num((ntoh32(iphdr->sip) >> 16) &0xff);
			sys_print_str(".");
			sys_print_num((ntoh32(iphdr->sip) >> 8) &0xff);
			sys_print_str(".");
			sys_print_num((ntoh32(iphdr->sip)) &0xff);
			sys_print_str(" ");
		}

		sys_print_num(recv_tick - send_tick);
		sys_print_str(" ms\n");

		if(icmphdr->type == ICMP_HDR_TYPE_ECHO_REPLY) {
			return;
		}
	}
}

void exec_httpget(char *buf)
{
	char host[100];
	char path[100];
	int i;
	int buf_len = strlen(buf);
	if(!strncmp("http://", buf, 7)) {
		sys_print_str("invalid format. use httpget http://hoge.com/fuga\n");
		return;
	}
	buf+=7;

	memset(host, 0, 100);
	memset(path, 0, 100);
	for(i = 0; i < buf_len; i++) {
		if(buf[i] == '/') {
			strncpy(host, buf, i);
			strncpy(path, buf + i, buf_len - i);
			break;
		}
	}
	if(path[0] == 0) {
		strncpy(host, buf, buf_len);
		path[0] = '/';
	}

	int udp_sock = sys_udp_socket();
	uint32_t ipaddr = resolve_addr(udp_sock, host);
	if(ipaddr == 0) {
		return;
	}

	sys_print_str("HTTPGET host: ");
	sys_print_str(host);
	sys_print_str(" (");
	sys_print_num((ipaddr >> 24) &0xff);
	sys_print_str(".");
	sys_print_num((ipaddr >> 16) &0xff);
	sys_print_str(".");
	sys_print_num((ipaddr >> 8) &0xff);
	sys_print_str(".");
	sys_print_num((ipaddr >> 0) &0xff);
	sys_print_str(")");
	sys_print_str(" path: ");
	sys_print_str(path);
	sys_print_str("\n");

	char _http_req[500];
	char *http_req = _http_req;
	char *http_req_head = http_req;
	memset(http_req, 0, 500);

	char *http_str1 = "GET ";
	char *http_str2 = " HTTP/1.1\r\nHOST:  ";
	char *http_str3 = "\r\n\r\n";

	strncpy(http_req, http_str1, strlen(http_str1));
	http_req += strlen(http_str1);

	strncpy(http_req, host, strlen(host));
	http_req += strlen(host);

	strncpy(http_req, http_str2, strlen(http_str2));
	http_req += strlen(http_str2);

	strncpy(http_req, path, strlen(path));
	http_req += strlen(path);

	strncpy(http_req, http_str3, strlen(http_str3));
	http_req += strlen(http_str3);

	int tcp_sock = sys_tcp_socket();

	int ret = sys_tcp_socket_connect(tcp_sock, ipaddr, 80);
	if(ret != 1) {
		ret = sys_tcp_socket_connect(tcp_sock, ipaddr, 80);
	}

	sys_tcp_socket_send(tcp_sock, (uint8_t *)http_req_head, strlen(http_req_head));

	char http_res[1600];
	while(1) {
		memset(http_res, 0, 1500);
		ret = sys_tcp_socket_recv(tcp_sock, (uint8_t *)http_res, 1500, 2000);
		sys_print_str(http_res);
		if(ret <= 0) {
			break;
		}
	}
	sys_tcp_socket_close(tcp_sock);
	return;
}

void exec_telnet(char *buf)
{
	uint32_t ipaddr = parse_ipaddr(buf);
	int udp_sock = sys_udp_socket();

	if(ipaddr == 0) {
		ipaddr = resolve_addr(udp_sock, buf);
	}

	if(ipaddr == 0) {
		return;
	}

	sys_print_str("telnet to ");
	sys_print_str(buf);
	sys_print_str(" (");
	sys_print_num((ipaddr >> 24) &0xff);
	sys_print_str(".");
	sys_print_num((ipaddr >> 16) &0xff);
	sys_print_str(".");
	sys_print_num((ipaddr >> 8) &0xff);
	sys_print_str(".");
	sys_print_num((ipaddr >> 0) &0xff);
	sys_print_str(")\n");

	int tcp_sock = sys_tcp_socket();

	int ret = sys_tcp_socket_connect(tcp_sock, ipaddr, 23);
	if(ret != 1) {
		ret = sys_tcp_socket_connect(tcp_sock, ipaddr, 23);
		if(ret != 1) {
			return;
		}
	}

	uint8_t read_buf[1600];
	memset(read_buf, 0, 1500);
	while(1) {
		ret = sys_tcp_socket_recv(tcp_sock, (uint8_t *)read_buf, 1500, 0);
		if(ret > 0) {
			int from = 0;
			int to = 0;
			//skip control chars
			for(from = 0; from < ret; from++) {
				if(read_buf[from] == 0xff) {
					from += 2;
				} else {
					read_buf[to] = read_buf[from];
					to++;
				}
			}
			read_buf[to] = 0;
			sys_print_str((char *)read_buf);
			memset(read_buf, 0, 1500);
		}
		if(ret < 0) {
			break;
		}
		char ch = sys_key_getc(0);
		if(ch != 0) {
			sys_tcp_socket_send(tcp_sock, (uint8_t *)&ch, 1);
		}
	}
	sys_tcp_socket_close(tcp_sock);
	return;
}

void exec_nc(char *buf)
{
	char host[100];
	char port[100];
	int i;
	int buf_len = strlen(buf);

	memset(host, 0, 100);
	memset(port, 0, 100);
	for(i = 0; i < buf_len; i++) {
		if(buf[i] == ' ') {
			strncpy(host, buf, i);
			strncpy(port, buf + i + 1, buf_len - i - 1);
			break;
		}
	}
	if(host[0] == 0){
		return;
	}

	int udp_sock = sys_udp_socket();
	uint32_t ipaddr = parse_ipaddr(host);
	if(ipaddr == 0) {
		ipaddr = resolve_addr(udp_sock, host);
	}
	if(ipaddr == 0) {
		return;
	}

	uint32_t dstport = stoi(port);
	if(dstport == 0) {
		sys_print_str("invalid port: ");
		sys_print_str(port);
		sys_print_str("\n");
		return;
	}

	sys_print_str("nc ");
	sys_print_str(host);
	sys_print_str(" (");
	sys_print_num((ipaddr >> 24) &0xff);
	sys_print_str(".");
	sys_print_num((ipaddr >> 16) &0xff);
	sys_print_str(".");
	sys_print_num((ipaddr >> 8) &0xff);
	sys_print_str(".");
	sys_print_num((ipaddr >> 0) &0xff);
	sys_print_str(")");
	sys_print_str(" : ");
	sys_print_num(dstport);
	sys_print_str("\n");

	int tcp_sock = sys_tcp_socket();

	int ret = sys_tcp_socket_connect(tcp_sock, ipaddr, dstport);
	if(ret != 1) {
		ret = sys_tcp_socket_connect(tcp_sock, ipaddr, dstport);
	}
	if(ret != 1) {
		return;
	}

	sys_print_str("Connected: type exit to exit nc command\n");

	int write_pos = 0;
	uint8_t write_buf[100];
	uint8_t read_buf[100];
	memset(write_buf, 0, 100);
	memset(read_buf, 0, 100);
	while(1) {
		ret = sys_tcp_socket_recv(tcp_sock, (uint8_t *)read_buf, 100, 0);
		if(ret > 0) {
			sys_print_str((char *)read_buf);
			memset(read_buf, 0, 100);
		}
		if(ret < 0) {
			break;
		}
		char ch = sys_key_getc(0);
		if(ch == 0){
			continue;
		}
		char tmp_buf[2];
		tmp_buf[0] = ch;
		tmp_buf[1] = 0;
		sys_print_str(tmp_buf);
		if(ch == 0x0a) {
			if(strlen((char *)write_buf) == 4 && strncmp((char *)write_buf, "exit", 4)) {
				break;
			}
			sys_tcp_socket_send(tcp_sock, (uint8_t *)write_buf, write_pos);
			write_pos = 0;
			memset(write_buf, 0, 100);
		} else {
			write_buf[write_pos] = ch;
			write_pos++;
			if(write_pos > 90) {
				sys_print_str("\ncommand too long\n");
				break;
			}
		}
	}
	sys_tcp_socket_close(tcp_sock);
	return;
}

//test for tcp tx packet loss
void exec_nctest()
{
	uint32_t ipaddr = (192 << 24) + (168 << 16) + (2 << 8) + 1;
	uint32_t dstport = 8888;

	sys_print_str("nctest\n");

	int tcp_sock = sys_tcp_socket();

	int ret = sys_tcp_socket_connect(tcp_sock, ipaddr, dstport);
	if(ret != 1) {
		ret = sys_tcp_socket_connect(tcp_sock, ipaddr, dstport);
	}
	if(ret != 1) {
		return;
	}

	sys_print_str("Connected: test start\n");

	int i, j;
	for(i = 0; i < 30; i++) {
		sys_print_str("i = ");
		sys_print_num(i);
		sys_print_str("\n");
		uint8_t buf[2];
		buf[0] = '0' + i%10;
		buf[1] = '\n';
		sys_tcp_socket_send(tcp_sock, (uint8_t *)buf, 2);
		for(j = 0; j < 500000000; j++) {
		}
	}

	sys_tcp_socket_close(tcp_sock);
	return;
}

void exec_ncserver(char *buf)
{
	int tcp_sock = sys_tcp_socket();
	uint32_t my_addr = (192 << 24) | (168 << 16) | (2 << 8) | 2;
	int ret;

	uint32_t port = stoi(buf);
	if(port == 0) {
		sys_print_str("invalid port: ");
		sys_print_str(buf);
		sys_print_str("\n");
		return;
	}

	sys_tcp_socket_bind(tcp_sock, my_addr, port);
	sys_tcp_socket_listen(tcp_sock);

	ret = sys_tcp_socket_accept(tcp_sock);
	if(ret == -1) {
		sys_print_str("failed to accept\n");
	} else {
		sys_print_str("tcp connected\n");
	}

	uint8_t readbuf[1505];
	while(1){
		memset(readbuf, 0, 1505);
		ret = sys_tcp_socket_recv(tcp_sock, (uint8_t *)readbuf, 1500, 1000);
		if(ret == 0) {
			sys_print_str("tcp_socket_recv timeout\n");
			continue;
		}
		if(ret == -1) {
			sys_print_str("tcp_socket_recv ret -1\n");
			break;
		}

		sys_print_str((char*) readbuf);
		sys_print_str("\n");
	}

	sys_tcp_socket_close(tcp_sock);
	return;
}

void exec_command(char *buf)
{
	if(strncmp("ping ", buf, 5)) {
		exec_ping(buf + 5);
	} else if(strncmp("traceroute ", buf, 11)) {
		exec_traceroute(buf + 11);
	} else if(strncmp("httpget ", buf, 8)) {
		exec_httpget(buf + 8);
	} else if(strncmp("telnet ", buf, 7)) {
		exec_telnet(buf + 7);
	} else if(strncmp("nc ", buf, 3)) {
		exec_nc(buf + 3);
	} else if(strncmp("nctest", buf, 6)) {
		exec_nctest();
	} else if(strncmp("ncserver ", buf, 9)) {
		exec_ncserver(buf + 9);
	} else if(strncmp("help", buf, 4)) {
		sys_print_str("ping dest (ipaddr/hostname)\n");
		sys_print_str("traceroute dest (ipaddr/hostname)\n");
		sys_print_str("httpget URL (http only)\n");
		sys_print_str("telnet dest (ipaddr/hostname)\n");
		sys_print_str("nc dest (ipaddr/hostname)\n");
		sys_print_str("nctest dest (ipaddr/hostname)\n");
		sys_print_str("ncserver port\n");
	} else {
		sys_print_str("command not found: ");
		sys_print_str(buf);
		sys_print_str("\n");
	}
}
