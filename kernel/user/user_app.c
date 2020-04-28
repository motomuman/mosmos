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
			char ch = sys_key_getc();
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

		exec_command(buf);
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
		ret = sys_tcp_socket_recv(tcp_sock, (uint8_t *)http_res, 1500);
		sys_print_str(http_res);
		if(ret <= 0) {
			break;
		}
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
	} else if(strncmp("help", buf, 4)) {
		sys_print_str("ping dest (ipaddr/hostname)\n");
		sys_print_str("traceroute dest (ipaddr/hostname)\n");
		sys_print_str("traceroute dest (ipaddr/hostname)\n");
		sys_print_str("httpget URL (http only)\n");
	} else {
		sys_print_str("command not found: ");
		sys_print_str(buf);
		sys_print_str("\n");
	}
}
