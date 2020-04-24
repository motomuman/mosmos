#include "dns.h"
#include "lib.h"
#include "netutil.h"
#include "user_syscall.h"

/*
 * Set DNS question section (domain name, type, class)
 * Return the size of question section
 */
uint32_t set_qsection(uint8_t *buf, char *name, uint16_t type, uint16_t class)
{
	uint8_t *buf_head = buf;
	int start = 0;
	int end = 0;
	uint32_t name_len = strlen(name);
	while(end < name_len) {
		end++;
		if(name[end] == '.') {
			//set next block length
			*buf = end - start;
			buf++;
			//set block
			memcpy(buf, name + start, end - start);
			buf += end - start;
			start = end + 1;
			end = end + 1;
		}
	}

	//set last block length
	*buf = end - start;
	buf++;
	//set last block
	memcpy(buf, name + start, end - start);
	buf += end - start;

	//set end of domain
	*buf = 0;
	buf++;

	*(uint16_t *)buf = hton16(type);
	buf += DNS_TYPE_LEN;
	*(uint16_t *)buf = hton16(class);
	buf += DNS_CLASS_LEN;

	return buf - buf_head;
}

int parse_domain_name(uint8_t *rbuf, uint8_t *wbuf, int wlen)
{

	while(*rbuf != 0) {
		if(*rbuf == 0xc0) {
			sys_print_str("ERROR can not parse offset\n");
			return -1;
		}
		int nextlen = *rbuf;
		rbuf+=1;
		wlen -= nextlen;
		if(wlen < 0) {
			sys_print_str("ERROR not enough space to write parsed domain name\n");
			return -1;
		}
		memcpy(wbuf, rbuf, nextlen);
		wbuf += nextlen;
		*wbuf = '.';
		wbuf += 1;
		wlen--;
		rbuf += nextlen;
	}
	return 0;
}

uint32_t resolve_addr(int sock, char *name)
{
	//uint32_t len = strlen(name);
	uint32_t resolver_ip = (8 << 24) | (8 << 16) | (8 << 8) | 8;

	//uint8_t *txbuf = (uint8_t*) mem_alloc(sizeof(struct dns_hdr) + len + 10, "dns query");
	//uint8_t *rxbuf = (uint8_t*) mem_alloc(500, "dns response");
	//uint8_t *txbuf_head = txbuf;
	//uint8_t *rxbuf_head = rxbuf;
	uint8_t _txbuf[500];
	uint8_t _rxbuf[500];
	uint8_t *txbuf = _txbuf;
	uint8_t *rxbuf = _rxbuf;
	memset(txbuf, 0, 500);
	memset(rxbuf, 0, 500);

	struct dns_hdr * dnshdr = (struct dns_hdr *) txbuf;
	dnshdr->id = hton16(777);
	//ask recursive, no dnssec
	dnshdr->opt = hton16(1<<8 | 1<<4);
	dnshdr->qdcount = hton16(1);
	dnshdr->ancount = 0;
	dnshdr->nscount = 0;
	dnshdr->arcount = 0;
	txbuf += sizeof(struct dns_hdr);

	//set question section
	int qsection_len = set_qsection(txbuf, name, DNS_TYPE_A, DNS_CLASS_IN);
	txbuf += qsection_len;

	//send to resolver ip
	txbuf -= (sizeof(struct dns_hdr) + qsection_len);
	sys_udp_socket_send(sock, resolver_ip, 53, txbuf, sizeof(struct dns_hdr) + qsection_len);

	int ret = sys_udp_socket_recv(sock, rxbuf, 500);
	if(ret == -1) {
		sys_print_str("Failed to resolve dns\n");
		return 0;
	}
	struct dns_hdr *ansdns_hdr = (struct dns_hdr *)rxbuf;
	rxbuf += sizeof(struct dns_hdr);

	/*
	 * Due to rough implementation.
	 * response handling is not perfect, ignoring many cases.
	 */
	if(ntoh16(ansdns_hdr->qdcount) != 1){
		//sys_print_str("Can not handle dns response with non one qd count (");
		//sys_print_num(ntoh16(ansdns_hdr->qdcount));
		//sys_print_str(") \n");
		//mem_free(txbuf_head);
		//mem_free(rxbuf_head);
		return 0;
	}

	// skip query section
	// not parsing, assuming qeury section is same to request
	rxbuf += qsection_len;

	if(*rxbuf != 0xc0){
		//sys_print_str("Can not handle dns response not using offset\n");
		//mem_free(txbuf_head);
		//mem_free(rxbuf_head);
		return 0;
	}
	
	//skip info in ans section
	rxbuf += DNS_OFFSET_LEN + DNS_TYPE_LEN + DNS_CLASS_LEN + DNS_TTL_LEN;

	uint16_t *length = (uint16_t *) (rxbuf);
	if(ntoh16(*length) != 4){
		//sys_print_str("Can not handle dns response with non 4 length\n");
		//mem_free(txbuf_head);
		//mem_free(rxbuf_head);
		return 0;
	}
	rxbuf += DNS_LENGTH_LEN;

	uint32_t resolved_ip = 0;
	resolved_ip = rxbuf[0];
	resolved_ip <<= 8;
	resolved_ip += rxbuf[1];
	resolved_ip <<= 8;
	resolved_ip += rxbuf[2];
	resolved_ip <<= 8;
	resolved_ip += rxbuf[3];

	//mem_free(txbuf_head);
	//mem_free(rxbuf_head);

	return resolved_ip;
}

int set_num(char *buf, uint8_t orinum) {
	uint8_t num = orinum;
	int ret = 0;
	if(num >= 100) {
		*buf = (num/100) + '0';
		num %= 100;
		buf++;
		ret++;
	}

	if(orinum >= 100 || num >= 10) {
		*buf = (num/10) + '0';
		num %= 10;
		buf++;
		ret++;
	}

	*buf = num + '0';
	buf++;
	ret++;

	return ret;
}

void generate_arpa_address(char *buf, uint32_t ip)
{
	uint8_t addr1 = (ip >> 0) & 0xff;
	uint8_t addr2 = (ip >> 8) & 0xff;
	uint8_t addr3 = (ip >> 16) & 0xff;
	uint8_t addr4 = (ip >> 24) & 0xff;

	int len = set_num(buf, addr1);
	buf+=len;
	*buf = '.';
	buf++;

	len = set_num(buf, addr2);
	buf+=len;
	*buf = '.';
	buf++;

	len = set_num(buf, addr3);
	buf+=len;
	*buf = '.';
	buf++;

	len = set_num(buf, addr4);
	buf+=len;
	*buf = '.';
	buf++;

	memcpy(buf, "in-addr.arpa", 12);
	buf[13] = 0;
	return;

}

int resolve_host(int sock, uint32_t ip, char *buf, int buflen)
{
	uint32_t resolver_ip = (8 << 24) | (8 << 16) | (8 << 8) | 8;

	//uint8_t *txbuf = (uint8_t*) mem_alloc(sizeof(struct dns_hdr) + 100, "dns query");
	//uint8_t *rxbuf = (uint8_t*) mem_alloc(500, "dns response");
	//uint8_t *txbuf_head = txbuf;
	//uint8_t *rxbuf_head = rxbuf;
	uint8_t _txbuf[500];
	uint8_t _rxbuf[500];
	uint8_t *txbuf = _txbuf;
	uint8_t *rxbuf = _rxbuf;
	memset(txbuf, 0, 500);
	memset(rxbuf, 0, 500);

	struct dns_hdr * dnshdr = (struct dns_hdr *) txbuf;
	dnshdr->id = hton16(777);
	//ask recursive, no dnssec
	dnshdr->opt = hton16(1<<8 | 1<<4);
	dnshdr->qdcount = hton16(1);
	dnshdr->ancount = 0;
	dnshdr->nscount = 0;
	dnshdr->arcount = 0;
	txbuf += sizeof(struct dns_hdr);

	// generate arpa address
	char arpa_domain[100];
	memset(arpa_domain, 0, 100);
	generate_arpa_address(arpa_domain, ip);
	int qsection_len = set_qsection(txbuf, arpa_domain, DNS_TYPE_PTR, DNS_CLASS_IN);
	txbuf += qsection_len;

	txbuf -= (sizeof(struct dns_hdr) + qsection_len);
	sys_udp_socket_send(sock, resolver_ip, 53, txbuf, sizeof(struct dns_hdr) + qsection_len);

	int ret = sys_udp_socket_recv(sock, rxbuf, 500);
	if(ret == -1) {
		//sys_print_str("udp_socket_recv: TIMEOUT\n");
		//mem_free(txbuf_head);
		//mem_free(rxbuf_head);
		return -1;
	}
	struct dns_hdr *ansdns_hdr = (struct dns_hdr *)rxbuf;
	rxbuf += sizeof(struct dns_hdr);

	/*
	 * Due to rough implementation.
	 * response handling is not perfect, ignoring many cases.
	 */
	if(ntoh16(ansdns_hdr->qdcount) != 1){
		//sys_print_str("Can not handle dns response with non one qd count (");
		//sys_print_num(ntoh16(ansdns_hdr->qdcount));
		//sys_print_str(") \n");
		//mem_free(txbuf_head);
		//mem_free(rxbuf_head);
		return -1;
	}

	if(ntoh16(ansdns_hdr->ancount) == 0){
		//sys_print_str("Can not handle dns response with zero ancount\n");
		//mem_free(txbuf_head);
		//mem_free(rxbuf_head);
		return -1;
	}

	// skip query section
	// not parsing, assuming qeury section is same to request
	rxbuf += qsection_len;

	if(rxbuf[0] != 0xc0){
		//sys_print_str("Can not handle dns response not using offset\n");
		//mem_free(txbuf_head);
		//mem_free(rxbuf_head);
		return -1;
	}

	ret = -1;
	uint16_t count = ntoh16(ansdns_hdr->ancount);
	while(count) {
		count--;
		if(*rxbuf != 0xc0){
			//sys_print_str("Can not handle dns response not using offset\n");
			//mem_free(txbuf_head);
			//mem_free(rxbuf_head);
			return -1;
		}
		rxbuf += DNS_OFFSET_LEN;
		uint16_t type = ntoh16(*(uint16_t *)rxbuf);

		rxbuf += DNS_TYPE_LEN + DNS_CLASS_LEN + DNS_TTL_LEN;
		uint16_t domain_len = ntoh16(*(uint16_t *)rxbuf);
		rxbuf += DNS_LENGTH_LEN;

		if(type != DNS_TYPE_PTR) {
			rxbuf += domain_len;
			continue;
		}
		
		ret = parse_domain_name(rxbuf, (uint8_t *)buf, buflen);
		//mem_free(txbuf_head);
		//mem_free(rxbuf_head);
		return ret;
	}

	//mem_free(txbuf_head);
	//mem_free(rxbuf_head);
	return ret;
}

