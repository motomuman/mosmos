#include "workqueue.h"
#include "link.h"
#include "task.h"
#include "memory.h"
#include "timer.h"

struct workqueue {
	struct listctl list;
	struct TASK *receiver_task;
	int cond;
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
	return;
}

void wq_set_receiver(struct TASK *receiver)
{
	wq.receiver_task = receiver;
	return;
}

void wq_push(void (*func) (void *), void *arg)
{
	struct work_task *task = (struct work_task *)mem_alloc(sizeof(struct work_task), "wq_task");
	task->func = func;
	task->arg = arg;
	list_pushback(&wq.list, &task->link);
	task_wakeup(&wq);
	return;
}

void wq_push_timer_func(void *_task)
{
	struct work_task *task = (struct work_task *) _task;
	list_pushback(&wq.list, &task->link);
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
		struct work_task *task = (struct work_task*)list_popfront(&wq.list);
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
