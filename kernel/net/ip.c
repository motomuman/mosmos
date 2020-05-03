#include "ip.h"
#include "print.h"
#include "netutil.h"
#include "pktbuf.h"
#include "icmp.h"
#include "netdev.h"
#include "arp.h"
#include "raw.h"
#include "udp.h"
#include "tcp.h"

#define NULL 0

/*
 * TODO support IP packet fragmentation
 */

void ip_rx(struct pktbuf *pkt)
{
	struct ip_hdr *iphdr = (struct ip_hdr *)pkt->buf;

	//if(iphdr->flafra != 0) {
	//	printstr_log("ignore fragmented ip");
	//	return;
	//}

	if(get_netdev()->ip_addr != ntoh32(iphdr->dip)) {
		//printstr_log("ignore ip packt to:");
		//printnum_log((ntoh32(iphdr->dip) >> 24) & 0xff);
		//printstr_log(".");
		//printnum_log((ntoh32(iphdr->dip) >> 16) & 0xff);
		//printstr_log(".");
		//printnum_log((ntoh32(iphdr->dip) >> 8) & 0xff);
		//printstr_log(".");
		//printnum_log((ntoh32(iphdr->dip) >> 0) & 0xff);
		//printstr_log("\n");
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
			tcp_rx(pkt);
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

	switch(proto) {
		case IP_HDR_PROTO_ICMP:
			printstr_log("ip_tx: ICMP\n");
			break;
		case IP_HDR_PROTO_TCP:
			printstr_log("ip_tx: TCP\n");
			break;
		case IP_HDR_PROTO_UDP:
			printstr_log("ip_tx: UDP\n");
			break;
		default:
			printstr_log("ip_tx: UNKNOWN proto\n");
			break;
	}

	uint8_t *mac_addr = find_mac_addr(nexthop_ip);
	if(mac_addr == NULL) {
		printstr_log("Send arp: ");
		printnum_log((nexthop_ip >> 24) & 0xff);
		printstr_log(".");
		printnum_log((nexthop_ip >> 16) & 0xff);
		printstr_log(".");
		printnum_log((nexthop_ip >> 8) & 0xff);
		printstr_log(".");
		printnum_log((nexthop_ip >> 0) & 0xff);
		printstr_log("\n");
		arp_tx(nexthop_ip);
	} else {
		pkt->buf -= sizeof(struct ether_hdr);
		ether_tx(pkt, mac_addr, ETHER_TYPE_IPV4);
	}
}

