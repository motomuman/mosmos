#ifndef _DNS_H_
#define _DNS_H_

#include "types.h"

/*
 * opt format
 *
 *   0   1       4  5    6    7    8    9   10   11  12   15
 * +--------------------------------------------------------+
 * | QR | Opcode  | AA | TC | RD | RA | Z | AD | CD | RCODE |
 * +--------------------------------------------------------+
 *
 * QR:		0 for query, 1 for response
 * Opcode:	0 for usual query
 * AA:		Authoritive Answer (server use)
 * TC:		Truncated (server use)
 * RD:		Recursion Desired.
 * RA:		Recursion Available (server use)
 * Z:		reserved (0)
 * AD:		Authenticated Data, DNSSEC
 * CD:		Checking Disabled, no dnssec
 * RCODE:	Return code (server use)
 *
 */

struct dns_hdr {
	uint16_t id;
	uint16_t opt;
	uint16_t qdcount;
	uint16_t ancount;
	uint16_t nscount;
	uint16_t arcount;
} __attribute__ ((packed));

#define DNS_TYPE_LEN 2
#define DNS_TYPE_A 1
#define DNS_TYPE_PTR 0x0c

#define DNS_CLASS_LEN 2
#define DNS_CLASS_IN 0x0001

#define DNS_OFFSET_LEN 2

#define DNS_TTL_LEN 4
#define DNS_LENGTH_LEN 2

uint32_t resolve_addr(int sock, char *name);
int resolve_host(int sock, uint32_t ip, char *buf, int buflen);

#endif
