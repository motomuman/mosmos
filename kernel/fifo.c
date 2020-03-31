#include "fifo.h"

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf)
{
	fifo->size = size;
	fifo->free = size;
	fifo->buf = buf;
	fifo->read_pos = 0;
	fifo->write_pos = 0;
	fifo->receive_task = NULL;
	return;
}

int fifo8_put(struct FIFO8 *fifo, unsigned char data)
{
	if(fifo->free == 0) {
		return -1;
	}
	fifo->buf[fifo->write_pos] = data;
	fifo->free--;
	fifo->write_pos++;
	if(fifo->write_pos == fifo->size){
		fifo->write_pos = 0;
	}
	if(fifo->receive_task != NULL) {
		task_run(fifo->receive_task);
	}
	return 0;
}

int fifo8_get(struct FIFO8 *fifo)
{
	int data;
	if (fifo->free == fifo->size) {
		return -1;
	}
	data = fifo->buf[fifo->read_pos];
	fifo->read_pos++;
	fifo->free++;
	if(fifo->read_pos == fifo->size){
		fifo->read_pos = 0;
	}
	return data;
}

int fifo8_status(struct FIFO8 *fifo)
{
	return fifo->size - fifo->free;
}
