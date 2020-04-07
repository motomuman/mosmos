#include "ip.h"
#include "print.h"
#include "netutil.h"
#include "pktbuf.h"
#include "icmp.h"
#include "netdev.h"
#include "arp.h"

#define NULL 0

void show_iphdr(struct ip_hdr *iphdr)
{
	printstr_app("ver_ihl: ");
	printhex_app(iphdr->ver_ihl);
	printstr_app("\n");

	printstr_app("tos: ");
	printhex_app(iphdr->tos);
	printstr_app("\n");

	printstr_app("len: ");
	printhex_app(ntoh16(iphdr->len));
	printstr_app("\n");

	printstr_app("id: ");
	printhex_app(ntoh16(iphdr->id));
	printstr_app("\n");

	printstr_app("flafra: ");
	printhex_app(ntoh16(iphdr->flafra));
	printstr_app("\n");

	printstr_app("ttl: ");
	printhex_app(iphdr->ttl);
	printstr_app("\n");

	printstr_app("proto: ");
	printhex_app(iphdr->proto);
	printstr_app("\n");

	printstr_app("sip: ");
	printnum_app((ntoh32(iphdr->sip) >> 24) & 0xff);
	printstr_app(".");
	printnum_app((ntoh32(iphdr->sip) >> 16) & 0xff);
	printstr_app(".");
	printnum_app((ntoh32(iphdr->sip) >> 8) & 0xff);
	printstr_app(".");
	printnum_app((ntoh32(iphdr->sip) >> 0) & 0xff);
	printstr_app("\n");

	printstr_app("dip: ");
	printnum_app((ntoh32(iphdr->dip) >> 24) & 0xff);
	printstr_app(".");
	printnum_app((ntoh32(iphdr->dip) >> 16) & 0xff);
	printstr_app(".");
	printnum_app((ntoh32(iphdr->dip) >> 8) & 0xff);
	printstr_app(".");
	printnum_app((ntoh32(iphdr->dip) >> 0) & 0xff);
	printstr_app("\n");
}

void ip_rx(struct pktbuf *pkt)
{
	struct ip_hdr *iphdr = (struct ip_hdr *)pkt->buf;
	pkt->buf += sizeof(struct ip_hdr);

	show_iphdr(iphdr);

	switch(iphdr->proto) {
		case IP_HDR_PROTO_ICMP:
			printstr_app("ip_rx: ICMP\n");
			icmp_rx(pkt, iphdr->sip);
			break;
		case IP_HDR_PROTO_TCP:
			printstr_app("ip_rx: TCP\n");
			break;
		case IP_HDR_PROTO_UDP:
			printstr_app("ip_rx: UDP\n");
			break;
		default:
			printstr_app("ip_rx: UNKNOWN proto\n");
			break;
	}
}

void ip_tx(struct pktbuf *pkt, uint32_t dip, uint8_t proto)
{
	struct ip_hdr *iphdr = (struct ip_hdr *)pkt->buf;


	iphdr->ver_ihl = (IP_HDR_PROTO_IPV4_VER << 4) | ((sizeof(struct ip_hdr)/4) & 0xf);
	iphdr->tos = 0;
	iphdr->len = hton16(pkt->pkt_len - sizeof(struct ether_hdr));
	iphdr->id = 0x77;
	iphdr->flafra = 0;
	iphdr->ttl = 64;
	iphdr->proto = proto;
	iphdr->cksum = 0;
	iphdr->sip = hton32(get_netdev()->ip_addr);
	iphdr->dip = hton32(dip);

	iphdr->cksum = checksum((uint16_t *)iphdr, sizeof(struct ip_hdr));

	uint8_t *mac_addr = find_mac_addr(dip);
	if(mac_addr == NULL) {
		printstr_app("ip_tx: don't know mac addr ignore\n");
	} else {
		printstr_app("ip_tx: I know mac addr send!\n");
		pkt->buf -= sizeof(struct ether_hdr);
		ether_tx(pkt, mac_addr, ETHER_TYPE_IPV4);
	}
}

