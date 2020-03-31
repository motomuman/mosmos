#ifndef _FIFO_H_
#define _FIFO_H_

#include "task.h"

struct FIFO8 {
	unsigned char *buf;
	int read_pos, write_pos, size, free;
	struct TASK *receive_task;
};

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf);
int fifo8_put(struct FIFO8 *fifo, unsigned char data);
int fifo8_get(struct FIFO8 *fifo);
int fifo8_status(struct FIFO8 *fifo);

#endif
