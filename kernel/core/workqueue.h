#ifndef _WQ_H_
#define _WQ_H_

#include <stdint.h>
#include "task.h"
#include "pktbuf.h"

enum worktype {
	wt_key_input = 1,
	wt_packet_receive = 2
};

struct work_key_input {
	uint8_t ch;
};

struct work_packet_receive {
	struct pktbuf *pbuf;
};

struct work {
	struct list_item link;
	enum worktype type;
	union {
		struct work_key_input key_input;
		struct work_packet_receive packet_receive;
	} u;
};

struct TASK;

void wq_init();
void wq_set_receiver(struct TASK *receiver);
void wq_push(struct work *w);
struct work* wq_pop();
int wq_empty();

#endif
