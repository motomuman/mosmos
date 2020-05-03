#include "arp.h"
#include "netutil.h"
#include "print.h"
#include "memory.h"
#include "netdev.h"
#include "ether.h"
#include "lib.h"

#define MAX_ARP_TABLE 256

struct arpentry {
	uint8_t mac_addr[ETHER_ADDR_LEN];
	uint32_t ip_addr;
};

struct {
	uint32_t next_entry_idx;
	struct arpentry table[MAX_ARP_TABLE];
} arptable;

void init_arptable()
{
	arptable.next_entry_idx = 0;
	printstr_log("Initialized ARP table\n");
}

void register_arpentry(uint8_t *mac_addr, uint32_t ip_addr)
{
	uint32_t entry_id = arptable.next_entry_idx;
	arptable.table[entry_id].ip_addr = ip_addr;
	memcpy(arptable.table[entry_id].mac_addr, mac_addr, ETHER_ADDR_LEN);
	arptable.next_entry_idx++;
}

uint8_t* find_mac_addr(uint32_t ip)
{
	int i;
	for(i = 0; i < arptable.next_entry_idx; i++) {
		if(arptable.table[i].ip_addr == ip) {
			return arptable.table[i].mac_addr;
		}
	}
	return 0;
}

void handle_arp_request(struct arp_etherip *arp)
{
	struct net_device *netdev = get_netdev();

	// This is arp for me. will sends response
	if(ntoh32(arp->dip) == netdev->ip_addr) {
		struct pktbuf * pkt = (struct pktbuf *)mem_alloc(sizeof(struct pktbuf), "arp_reply_pbuf");
		pkt->pkt_len = sizeof(struct ether_hdr) + sizeof(struct arp_hdr) + sizeof(struct arp_etherip);

		uint8_t *buf = (uint8_t *)mem_alloc(sizeof(uint8_t) * pkt->pkt_len, "arp_reply_pbuf_buf");
		pkt->buf = buf;
		pkt->buf_head = buf;

		// reserve for ether header
		pkt->buf += sizeof(struct ether_hdr);

		// set arp header
		struct arp_hdr *arphdr = (struct arp_hdr *) pkt->buf;
		arphdr->hard_type = hton16(ARP_HARD_TYPE_ETHER);
		arphdr->proto = hton16(ARP_PROTO_TCPIP);
		arphdr->hlen = ARP_HLEN_ETHER;
		arphdr->plen = ARP_PLEN_ETHER;
		arphdr->opcode = hton16(ARP_OP_RESPONSE);

		pkt->buf += sizeof(struct arp_hdr);

		// set arp contents
		struct arp_etherip *arpres = (struct arp_etherip *) pkt->buf;
		memcpy(arpres->dmac, arp->smac, ETHER_ADDR_LEN);
		arpres->dip = arp->sip;
		memcpy(arpres->smac, netdev->hw_addr, ETHER_ADDR_LEN);
		arpres->sip = hton32(netdev->ip_addr);

		// restore buf position
		pkt->buf -= sizeof(struct arp_hdr);
		pkt->buf -= sizeof(struct ether_hdr);

		// send arp response
		ether_tx(pkt, arp->smac, ETHER_TYPE_ARP);

		// register arp request source ip and mac
		register_arpentry(arp->smac, ntoh32(arp->sip));

		mem_free(pkt->buf_head);
		mem_free(pkt);
	}
}

void handle_arp_response(struct arp_etherip *arp)
{
	register_arpentry(arp->smac, ntoh32(arp->sip));
}

void arp_rx(struct pktbuf *pkt)
{
	printstr_log("ARP RX\n");
	struct arp_hdr *arphdr = (struct arp_hdr *)pkt->buf;
	pkt->buf += sizeof(struct arp_hdr);
	struct arp_etherip *arp = (struct arp_etherip *)pkt->buf;
	if (ntoh16(arphdr->hard_type) != ARP_HARD_TYPE_ETHER ||
		ntoh16(arphdr->proto)  != ARP_PROTO_TCPIP ||
		arphdr->hlen != ARP_HLEN_ETHER ||
		arphdr->plen != ARP_PLEN_ETHER) {
		printstr_log("invalid arp packet\n");
		return;
	}

	switch(ntoh16(arphdr->opcode)) {
		case ARP_OP_REQUEST:
			handle_arp_request(arp);
			break;
		case ARP_OP_RESPONSE:
			handle_arp_response(arp);
			break;
		default:
			printstr_log("unknown arp opcode\n");
			break;
	}
}

void arp_tx(uint32_t dip)
{
	struct net_device *netdev = get_netdev();

	struct pktbuf * pkt = (struct pktbuf *)mem_alloc(sizeof(struct pktbuf), "arp_req_pbuf");
	pkt->pkt_len = sizeof(struct ether_hdr) + sizeof(struct arp_hdr) + sizeof(struct arp_etherip);

	uint8_t *buf = (uint8_t *)mem_alloc(sizeof(uint8_t) * pkt->pkt_len, "arp_req_pbuf_buf");
	pkt->buf = buf;
	pkt->buf_head = buf;

	// reserve for ether header
	pkt->buf += sizeof(struct ether_hdr);

	// set arp header
	struct arp_hdr *arphdr = (struct arp_hdr *) pkt->buf;
	arphdr->hard_type = hton16(ARP_HARD_TYPE_ETHER);
	arphdr->proto = hton16(ARP_PROTO_TCPIP);
	arphdr->hlen = ARP_HLEN_ETHER;
	arphdr->plen = ARP_PLEN_ETHER;
	arphdr->opcode = hton16(ARP_OP_REQUEST);

	pkt->buf += sizeof(struct arp_hdr);

	// set arp contents
	struct arp_etherip *arpreq = (struct arp_etherip *) pkt->buf;
	memset(arpreq->dmac, 0xff, ETHER_ADDR_LEN);
	arpreq->dip = hton32(dip);
	memcpy(arpreq->smac, netdev->hw_addr, ETHER_ADDR_LEN);
	arpreq->sip = hton32(netdev->ip_addr);

	// restore buf position
	pkt->buf -= sizeof(struct arp_hdr);
	pkt->buf -= sizeof(struct ether_hdr);

	// send arp request
	ether_tx(pkt, arpreq->dmac, ETHER_TYPE_ARP);

	mem_free(pkt->buf_head);
	mem_free(pkt);
}
