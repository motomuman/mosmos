#include "udp.h"
#include "print.h"
#include "netutil.h"
#include "lib.h"
#include "memory.h"

void udp_rx(struct pktbuf *rxpkt)
{
	printstr_app("udp_rx\n");

	struct udp_hdr *udphdr = (struct udp_hdr *)rxpkt->buf;
	rxpkt->buf += sizeof(struct udp_hdr);

	printstr_app("udp sport: ");
	printnum_app(ntoh16(udphdr->sport));

	printstr_app(" dport: ");
	printnum_app(ntoh16(udphdr->dport));

	printstr_app(" len: ");
	printnum_app(ntoh16(udphdr->len));

	printstr_app(" cksum: ");
	printnum_app(ntoh16(udphdr->checksum));
	printstr_app("\n");

	uint32_t data_len = ntoh16(udphdr->len) - sizeof(struct udp_hdr);
	uint8_t *buf = (uint8_t *) mem_alloc(data_len + 10, "udp_rx_buf");
	memset(buf, 0, data_len + 10); 
	memcpy(buf, rxpkt->buf, data_len);
	printstr_app("data: ");
	printstr_app((char*) buf);
	printstr_app("\n");
	mem_free(buf);
}
