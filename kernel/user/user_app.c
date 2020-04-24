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

			tmp_buf[0] = ch;
			tmp_buf[1] = 0;
			sys_print_str(tmp_buf);
			if(ch == 0x0a) {
				break;
			}
			buf[buf_pos] = ch;
			buf_pos++;
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
		icmphdr->type = ICMP_HDR_TYPE_ECHO_REQUEST;
		icmphdr->code = 0;
		icmphdr->checksum = 0;
		icmphdr->echo.id = seq;
		icmphdr->echo.seqnum = seq;

		//set checksum
		icmphdr->checksum = checksum(buf, sizeof(struct icmp_hdr));

		sys_raw_socket_send(udp_sock, ipaddr, buf, sizeof(struct icmp_hdr), 64);

		int ret = sys_raw_socket_recv(raw_sock, buf, sizeof(struct icmp_hdr) + sizeof(struct ip_hdr));

		if(ret == -1) {
			sys_print_str("timeout!\n");
			continue;
		}
		struct ip_hdr *iphdr = (struct ip_hdr *)buf;
		sys_print_str("seq=");
		sys_print_num(seq);
		sys_print_str(" ttl=");
		sys_print_num(iphdr->ttl);
		sys_print_str("\n");
	}
}



void exec_command(char *buf)
{
	if(strncmp("ping ", buf, 5)) {
		exec_ping(buf + 5);
	} else if(strncmp("traceroute ", buf, 5)) {
		sys_print_str("traceroute command\n");
	} else if(strncmp("help", buf, 4)) {
		sys_print_str("ping dest (ipaddr/hostname)\n");
		sys_print_str("traceroute dest (ipaddr/hostname)\n");
	} else {
		sys_print_str("command not found: ");
		sys_print_str(buf);
		sys_print_str("\n");
	}
}

//void userland_main() {
//	int i;
//	int ttl = 1;
//	int sock = sys_raw_socket(IP_HDR_PROTO_ICMP);
//	int udp_sock = sys_udp_socket();
//	uint8_t _buf[500];
//	uint8_t *buf = _buf;
//	sys_print_str("sock: ");
//	sys_print_num(sock);
//	sys_print_str("\n");
//
//	while(1) {
//		for(i = 0; i < 1000000000; i++){
//		}
//
//		struct icmp_hdr *icmphdr = (struct icmp_hdr *) buf;
//		icmphdr->type = ICMP_HDR_TYPE_ECHO_REQUEST;
//		icmphdr->code = 0;
//		icmphdr->checksum = 0;
//		icmphdr->echo.id = 7777;
//		icmphdr->echo.seqnum = 0;
//
//		//set checksum
//		icmphdr->checksum = checksum(buf, sizeof(struct icmp_hdr));
//
//		uint32_t dip = (8 << 24) | (8 << 16) | (8 << 8) | 8;
//		sys_raw_socket_send(sock, dip, buf, sizeof(struct icmp_hdr), ttl);
//
//		int ret = sys_raw_socket_recv(sock, buf, sizeof(struct icmp_hdr) + sizeof(struct ip_hdr));
//		if(ret == -1) {
//			sys_print_str("TIMEOUT!!!!\n");
//			//raw_socket_free(sock);
//			continue;
//		}
//		sys_print_str("TTL: ");
//		sys_print_num(ttl);
//		sys_print_str("\n");
//		ttl++;
//		struct ip_hdr *iphdr = (struct ip_hdr *)buf;
//
//		char domainbuf[200];
//		memset(domainbuf, 0, 200);
//		ret = resolve_host(udp_sock, ntoh32(iphdr->sip) , domainbuf, 200);
//		if(ret != -1){
//			printstr_app(domainbuf);
//			sys_print_str(domainbuf);
//			sys_print_str("(");
//			sys_print_num((ntoh32(iphdr->sip) >> 24) &0xff);
//			sys_print_str(".");
//			sys_print_num((ntoh32(iphdr->sip) >> 16) &0xff);
//			sys_print_str(".");
//			sys_print_num((ntoh32(iphdr->sip) >> 8) &0xff);
//			sys_print_str(".");
//			sys_print_num((ntoh32(iphdr->sip)) &0xff);
//			sys_print_str(")\n");
//		} else {
//			sys_print_num((ntoh32(iphdr->sip) >> 24) &0xff);
//			sys_print_str(".");
//			sys_print_num((ntoh32(iphdr->sip) >> 16) &0xff);
//			sys_print_str(".");
//			sys_print_num((ntoh32(iphdr->sip) >> 8) &0xff);
//			sys_print_str(".");
//			sys_print_num((ntoh32(iphdr->sip)) &0xff);
//			sys_print_str("\n");
//		}
//
//		buf += sizeof(struct ip_hdr);
//		icmphdr = (struct icmp_hdr *)buf;
//		if(icmphdr->type == ICMP_HDR_TYPE_ECHO_REPLY) {
//			sys_print_str("ICMP_HDR_TYPE_ECHO_REPLY\n");
//		} else if(icmphdr->type == ICMP_HDR_TYPE_ECHO_REQUEST) {
//			sys_print_str("ICMP_HDR_TYPE_ECHO_REQUEST\n");
//		} else if (icmphdr->type ==  ICMP_HDR_TYPE_TIME_EXCEEDED){
//			sys_print_str("ICMP_HDR_TYPE_TIME_EXCEEDED\n");
//		}else{
//			sys_print_str("ICMP_HDR_TYPE_ECHO_INVALID\n");
//		}
//
//		buf -= sizeof(struct ip_hdr);
//
//	}
//	//raw_socket_free(sock);
//}

