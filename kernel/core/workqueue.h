#ifndef _WQ_H_
#define _WQ_H_

#include "types.h"

struct TASK;

void wq_init();
void wq_set_receiver(struct TASK *receiver);
void wq_push(void (*func) (void *), void *arg);
void wq_push_with_delay(void (*func) (void *), void *arg, uint32_t delay_msec);
void wq_execute();
int wq_empty();
void *wq_cond();

#endif
