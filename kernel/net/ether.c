#include "print.h"
#include "pktbuf.h"
#include "ether.h"
#include "netutil.h"
#include "arp.h"
#include "netdev.h"
#include "lib.h"
#include "ip.h"

void ether_rx(void *_pkt)
{
	struct pktbuf *pkt = (struct pktbuf *) _pkt;
	struct ether_hdr *ehdr = (struct ether_hdr *)pkt->buf;

	switch(ntoh16(ehdr->type)) {
		case ETHER_TYPE_ARP:
			pkt->buf += sizeof(struct ether_hdr);
			arp_rx(pkt);
			break;
		case ETHER_TYPE_IPV4:
			pkt->buf += sizeof(struct ether_hdr);
			ip_rx(pkt);
			break;
		default:
			printstr_log("unknown ether type: ");
			printhex_log(ntoh16(ehdr->type));
			printstr_log("\n");
			break;
	}
}

void ether_tx(struct pktbuf *pkt, uint8_t *dst_mac, uint16_t proto) {
	struct ether_hdr *ehdr = (struct ether_hdr *)pkt->buf;
	struct net_device *netdev = get_netdev();

	memcpy(ehdr->dmac, dst_mac, ETHER_ADDR_LEN);
	memcpy(ehdr->smac, netdev->hw_addr, ETHER_ADDR_LEN);
	ehdr->type = hton16(proto);
	netdev->tx(pkt);
}
