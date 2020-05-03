#ifndef _PKTBUF_H_
#define _PKTBUF_H_

#include "types.h"

struct pktbuf {
	uint32_t pkt_len;
	uint8_t *buf;
	uint8_t *buf_head;
};

#endif
