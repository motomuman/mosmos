#ifndef _WQ_H_
#define _WQ_H_

struct TASK;

void wq_init();
void wq_set_receiver(struct TASK *receiver);
void wq_push(void (*func) (void *), void *arg);
void wq_execute();
int wq_empty();

#endif
