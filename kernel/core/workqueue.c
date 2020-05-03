#include "workqueue.h"
#include "list.h"
#include "task.h"
#include "memory.h"
#include "timer.h"
#include "asm.h"
#include "print.h"

struct workqueue {
	struct listctl list;
};

struct work_task {
	struct list_item link;
	void (*func) (void *);
	void *arg;
};

struct workqueue wq;

void wq_init()
{
	list_init(&wq.list);
	printstr_log("Initialized workqueue\n");
	return;
}

void wq_push(void (*func) (void *), void *arg)
{
	struct work_task *task = (struct work_task *)mem_alloc(sizeof(struct work_task), "wq_task");
	task->func = func;
	task->arg = arg;

	uint64_t rflags = get_rflags();
	io_cli();
	list_pushback(&wq.list, &task->link);
	set_rflags(rflags);

	task_wakeup(&wq);
	return;
}

void wq_push_timer_func(void *_task)
{
	struct work_task *task = (struct work_task *) _task;

	uint64_t rflags = get_rflags();
	io_cli();
	list_pushback(&wq.list, &task->link);
	set_rflags(rflags);

	task_wakeup(&wq);
	return;
}

void wq_push_with_delay(void (*func) (void *), void *arg, uint32_t delay_msec)
{
	struct work_task *task = (struct work_task *)mem_alloc(sizeof(struct work_task), "delaoed_wq_task");
	task->func = func;
	task->arg = arg;
	set_timer(wq_push_timer_func, task, delay_msec);
	return;
}

void wq_execute()
{
	if(!wq_empty()) {
		uint64_t rflags = get_rflags();
		io_cli();
		struct work_task *task = (struct work_task*)list_popfront(&wq.list);
		set_rflags(rflags);

		task->func(task->arg);
		mem_free(task);
	}
}

int wq_empty()
{
	return list_empty(&wq.list);
}

void *wq_cond()
{
	return &wq;
}
