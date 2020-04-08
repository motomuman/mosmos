#include "icmp.h"
#include "print.h"
#include "netutil.h"
#include "memory.h"
#include "ether.h"
#include "ip.h"
#include "lib.h"

void icmp_rx(struct pktbuf *rxpkt, uint32_t sip)
{
	struct icmp_hdr *rxicmphdr = (struct icmp_hdr *)rxpkt->buf;
	rxpkt->buf += sizeof(struct icmp_hdr);

	if(rxicmphdr->type == ICMP_HDR_TYPE_ECHO_REQUEST) {
		printstr_app("icmp_rx: sends reply\n");
		struct icmp_echo *rxecho = (struct icmp_echo *)rxpkt->buf;
		rxpkt->buf += sizeof(struct icmp_echo);

		// Build icmp echo reply and send
		struct pktbuf * txpkt = (struct pktbuf *)mem_alloc(sizeof(struct pktbuf), "icmp_reply_pbuf");
		txpkt->pkt_len = rxpkt->pkt_len;

		uint8_t *buf = (uint8_t *)mem_alloc(sizeof(uint8_t) * txpkt->pkt_len, "icmp_reply_pbuf_buf");
		txpkt->buf = buf;
		txpkt->buf_head = buf;

		// reserve for ether header and ip_hdr
		txpkt->buf += sizeof(struct ether_hdr) + sizeof(struct ip_hdr);

		// set icmp header
		struct icmp_hdr *icmphdr = (struct icmp_hdr *) txpkt->buf;
		txpkt->buf += sizeof(struct icmp_hdr);
		icmphdr->type = ICMP_HDR_TYPE_ECHO_REPLY;
		icmphdr->code = 0;
		icmphdr->checksum = 0;


		// set icmp contents
		struct icmp_echo *txecho = (struct icmp_echo *) txpkt->buf;
		txpkt->buf += sizeof(struct icmp_echo);
		txecho->id = rxecho->id;
		txecho->seqnum = rxecho->seqnum;

		memcpy(txpkt->buf, rxpkt->buf,
				txpkt->pkt_len - (sizeof(struct ether_hdr) + sizeof(struct ip_hdr)
							+ sizeof(struct icmp_hdr) + sizeof(struct icmp_echo)));

		// restore buf position
		txpkt->buf -= sizeof(struct icmp_echo);
		txpkt->buf -= sizeof(struct icmp_hdr);

		//set checksum
		icmphdr->checksum = checksum(txpkt->buf, txpkt->pkt_len - (sizeof(struct ether_hdr) + sizeof(struct ip_hdr)));


		txpkt->buf -= sizeof(struct ip_hdr);

		// send icmp reply
		ip_tx(txpkt, ntoh32(sip), IP_HDR_PROTO_ICMP);

		mem_free(txpkt->buf_head);
		mem_free(txpkt);
	}
}

void icmp_tx(uint32_t dip)
{
	printstr_app("icmp_tx: sends echo request\n");

	// Build icmp echo reply and send
	struct pktbuf * txpkt = (struct pktbuf *)mem_alloc(sizeof(struct pktbuf), "icmp_tx_pbuf");
	txpkt->pkt_len =  (sizeof(struct ether_hdr) + sizeof(struct ip_hdr)
				+ sizeof(struct icmp_hdr) + sizeof(struct icmp_echo));

	uint8_t *buf = (uint8_t *)mem_alloc(sizeof(uint8_t) * txpkt->pkt_len, "icmp_tx_pbuf_buf");
	txpkt->buf = buf;
	txpkt->buf_head = buf;

	// reserve for ether header and ip_hdr
	txpkt->buf += sizeof(struct ether_hdr) + sizeof(struct ip_hdr);

	// set icmp header
	struct icmp_hdr *icmphdr = (struct icmp_hdr *) txpkt->buf;
	txpkt->buf += sizeof(struct icmp_hdr);
	icmphdr->type = ICMP_HDR_TYPE_ECHO_REQUEST;
	icmphdr->code = 0;
	icmphdr->checksum = 0;

	// set icmp contents
	struct icmp_echo *txecho = (struct icmp_echo *) txpkt->buf;
	txpkt->buf += sizeof(struct icmp_echo);
	txecho->id = 7777;
	txecho->seqnum = 0;

	// TODO write icmp data


	// restore buf position
	txpkt->buf -= sizeof(struct icmp_echo);
	txpkt->buf -= sizeof(struct icmp_hdr);

	//set checksum
	icmphdr->checksum = checksum(txpkt->buf, txpkt->pkt_len - (sizeof(struct ether_hdr) + sizeof(struct ip_hdr)));

	txpkt->buf -= sizeof(struct ip_hdr);

	// send icmp reply
	ip_tx(txpkt, dip, IP_HDR_PROTO_ICMP);

	mem_free(txpkt->buf_head);
	mem_free(txpkt);
}
