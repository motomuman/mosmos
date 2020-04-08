#ifndef _TIMER_H_
#define _TIMER_H_

void init_pit();
void init_timer();
void set_timer(void (*func) (void *), void *arg, uint32_t time_msec);

#endif
