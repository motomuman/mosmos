#include "ip.h"
#include "print.h"
#include "netutil.h"
#include "pktbuf.h"
#include "icmp.h"
#include "netdev.h"
#include "arp.h"
#include "raw.h"
#include "udp.h"

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

	if(iphdr->flafra != 0) {
		printstr_log("ignore fragmented ip");
		return;
	}

	if(get_netdev()->ip_addr != ntoh32(iphdr->dip)) {
		printstr_log("ignore ip packt to:");
		printnum_log((ntoh32(iphdr->dip) >> 24) & 0xff);
		printstr_log(".");
		printnum_log((ntoh32(iphdr->dip) >> 16) & 0xff);
		printstr_log(".");
		printnum_log((ntoh32(iphdr->dip) >> 8) & 0xff);
		printstr_log(".");
		printnum_log((ntoh32(iphdr->dip) >> 0) & 0xff);
		printstr_log("\n");
		return;
	}

	raw_recv(pkt, iphdr->proto);
	pkt->buf += sizeof(struct ip_hdr);

	switch(iphdr->proto) {
		case IP_HDR_PROTO_ICMP:
			printstr_log("ip_rx: ICMP\n");
			icmp_rx(pkt, iphdr->sip);
			break;
		case IP_HDR_PROTO_TCP:
			printstr_log("ip_rx: TCP\n");
			break;
		case IP_HDR_PROTO_UDP:
			printstr_log("ip_rx: UDP\n");
			udp_rx(pkt);
			break;
		default:
			printstr_log("ip_rx: UNKNOWN proto\n");
			break;
	}
}

void ip_tx(struct pktbuf *pkt, uint32_t dip, uint8_t proto, uint8_t ttl)
{
	struct ip_hdr *iphdr = (struct ip_hdr *)pkt->buf;
	struct net_device *netdev = get_netdev();

	iphdr->ver_ihl = (IP_HDR_PROTO_IPV4_VER << 4) | ((sizeof(struct ip_hdr)/4) & 0xf);
	iphdr->tos = 0;
	iphdr->len = hton16(pkt->pkt_len - sizeof(struct ether_hdr));
	iphdr->id = 0x77;
	iphdr->flafra = 0;
	iphdr->ttl = ttl;
	iphdr->proto = proto;
	iphdr->cksum = 0;
	iphdr->sip = hton32(netdev->ip_addr);
	iphdr->dip = hton32(dip);

	iphdr->cksum = checksum((uint16_t *)iphdr, sizeof(struct ip_hdr));

	uint32_t nexthop_ip;
	if((dip >> (32 - netdev->netmask)) == (netdev->ip_addr >> (32 - netdev->netmask))) {
		nexthop_ip = dip;
	} else {
		nexthop_ip = netdev->gw_addr;
	}

	uint8_t *mac_addr = find_mac_addr(nexthop_ip);
	if(mac_addr == NULL) {
		printstr_log("ip_tx: don't know mac addr, send arp\n");
		arp_tx(nexthop_ip);
	} else {
		printstr_log("ip_tx: I know mac addr send!\n");
		pkt->buf -= sizeof(struct ether_hdr);
		ether_tx(pkt, mac_addr, ETHER_TYPE_IPV4);
	}
}

