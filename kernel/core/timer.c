#include <stdint.h>
#include "int.h"
#include "list.h"
#include "memory.h"
#include "print.h"
#include "task.h"
#include "lib.h"
#include "asm.h"

struct timer_entry {
	struct list_item link;
	uint32_t tick;
	void (*func) (void *);
	void *arg;
};
//
struct listctl timer_list;
uint32_t tick;
//
//void show_timer()
//{
//	struct timer_entry *timer;
//
//	printstr_app("cur t: ");
//	for(timer = (struct timer_entry *) list_head(&timer_list); timer != NULL; timer = (struct timer_entry *)list_next(&timer->link)) {
//		printnum_app(timer->tick);
//		printstr_app("(");
//		printhex_app((uint32_t)timer);
//		printstr_app(")");
//		printstr_app(":");
//	}
//	printstr_app("\n");
//}
//
// O(n)
void set_timer(void (*func) (void *), void *arg, uint32_t time_msec)
{
	struct timer_entry *new_timer = (struct timer_entry *)mem_alloc(sizeof(struct timer_entry), "set_timer");
	new_timer->func = func;
	new_timer->arg = arg;
	new_timer->tick = tick + time_msec/10;

	struct timer_entry *prev_timer = (struct timer_entry *) list_head(&timer_list);
	if(new_timer->tick < prev_timer->tick) {
		list_pushfront(&timer_list, &new_timer->link);
		return;
	}

	while(1) {
		struct timer_entry *next_timer = (struct timer_entry *) list_next(&prev_timer->link);
		if(next_timer == NULL || next_timer->tick > new_timer->tick) {
			break;
		}
		prev_timer = next_timer;
	}

	list_insert(&timer_list, &prev_timer->link, &new_timer->link);

	return;
}

#define NULL 0

void int_pit(int *esp) {
	pic_sendeoi(PIT_IRQ);
	tick++;
	if(tick%200 == 0) {
		printstr_log("int_pit, args addr ");
		printhex_log((uint64_t) &esp);
		printstr_log("\n");
		printstr_log("task_switch ");
		task_show();
		task_switch();
	}

	while(!list_empty(&timer_list) &&
			((struct timer_entry*)list_head(&timer_list))->tick <= tick) {
		struct timer_entry* timer = (struct timer_entry*)list_popfront(&timer_list);
		timer->func(timer->arg);
		mem_free(timer);
	}

	return;
}

void init_pit()
{
	tick = 0;
	/*     2bit           2bit         3bit    1bit
	 * |access counter| access method | mode | bcd |
	 */
	io_out8(0x0043, 0b00110100);

	//0x2e9c (11932) for 100Hz (interrupt every 10ms)
	io_out8(0x0040, 0x9c); // lower 8 bit
	io_out8(0x0040, 0x2e); // higher 8 bit

	register_interrupt(0, asm_int_pit);
	return;
}

void init_timer()
{
	list_init(&timer_list);

	struct timer_entry *last_timer = (struct timer_entry *)mem_alloc(sizeof(struct timer_entry), "last_timer");
	last_timer->func = NULL;
	last_timer->arg = NULL;
	last_timer->tick = 0;
	last_timer->tick = ~(last_timer->tick);
	list_pushback(&timer_list, &last_timer->link);

	return;
}
