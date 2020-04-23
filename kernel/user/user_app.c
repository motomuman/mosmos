#include <stdint.h>
#include "user_syscall.h"

#include "netutil.h"
#include "ip.h"
#include "print.h"
#include "icmp.h"
#include "dns.h"
#include "lib.h"

void userland_main() {
	int i;
	int ttl = 1;
	int sock = sys_raw_socket(IP_HDR_PROTO_ICMP);
	int udp_sock = sys_udp_socket();
	uint8_t _buf[500];
	uint8_t *buf = _buf;
	sys_print_str("sock: ");
	sys_print_num(sock);
	sys_print_str("\n");

	while(1) {
		for(i = 0; i < 1000000000; i++){
		}

		struct icmp_hdr *icmphdr = (struct icmp_hdr *) buf;
		icmphdr->type = ICMP_HDR_TYPE_ECHO_REQUEST;
		icmphdr->code = 0;
		icmphdr->checksum = 0;
		icmphdr->echo.id = 7777;
		icmphdr->echo.seqnum = 0;

		//set checksum
		icmphdr->checksum = checksum(buf, sizeof(struct icmp_hdr));

		uint32_t dip = (8 << 24) | (8 << 16) | (8 << 8) | 8;
		sys_raw_socket_send(sock, dip, buf, sizeof(struct icmp_hdr), ttl);

		int ret = sys_raw_socket_recv(sock, buf, sizeof(struct icmp_hdr) + sizeof(struct ip_hdr));
		if(ret == -1) {
			sys_print_str("TIMEOUT!!!!\n");
			//raw_socket_free(sock);
			continue;
		}
		sys_print_str("TTL: ");
		sys_print_num(ttl);
		sys_print_str("\n");
		ttl++;
		struct ip_hdr *iphdr = (struct ip_hdr *)buf;

		char domainbuf[200];
		memset(domainbuf, 0, 200);
		ret = resolve_host(udp_sock, ntoh32(iphdr->sip) , domainbuf, 200);
		if(ret != -1){
			printstr_app(domainbuf);
			sys_print_str(domainbuf);
			sys_print_str("(");
			sys_print_num((ntoh32(iphdr->sip) >> 24) &0xff);
			sys_print_str(".");
			sys_print_num((ntoh32(iphdr->sip) >> 16) &0xff);
			sys_print_str(".");
			sys_print_num((ntoh32(iphdr->sip) >> 8) &0xff);
			sys_print_str(".");
			sys_print_num((ntoh32(iphdr->sip)) &0xff);
			sys_print_str(")\n");
		} else {
			sys_print_num((ntoh32(iphdr->sip) >> 24) &0xff);
			sys_print_str(".");
			sys_print_num((ntoh32(iphdr->sip) >> 16) &0xff);
			sys_print_str(".");
			sys_print_num((ntoh32(iphdr->sip) >> 8) &0xff);
			sys_print_str(".");
			sys_print_num((ntoh32(iphdr->sip)) &0xff);
			sys_print_str("\n");
		}

		buf += sizeof(struct ip_hdr);
		icmphdr = (struct icmp_hdr *)buf;
		if(icmphdr->type == ICMP_HDR_TYPE_ECHO_REPLY) {
			sys_print_str("ICMP_HDR_TYPE_ECHO_REPLY\n");
		} else if(icmphdr->type == ICMP_HDR_TYPE_ECHO_REQUEST) {
			sys_print_str("ICMP_HDR_TYPE_ECHO_REQUEST\n");
		} else if (icmphdr->type ==  ICMP_HDR_TYPE_TIME_EXCEEDED){
			sys_print_str("ICMP_HDR_TYPE_TIME_EXCEEDED\n");
		}else{
			sys_print_str("ICMP_HDR_TYPE_ECHO_INVALID\n");
		}

		buf -= sizeof(struct ip_hdr);

	}
	//raw_socket_free(sock);
}

