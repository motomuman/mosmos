#ifndef _INT_H_
#define _INT_H_

#include "types.h"

#define PIT_IRQ 0
#define KEYBOARD_IRQ 1

void init_interrupt();
void register_interrupt(int irq, void * handler);
void pic_sendeoi(int irq);

#endif
