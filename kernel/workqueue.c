#include "workqueue.h"
#include "link.h"
#include "task.h"

struct workqueue {
	struct listctl list;
	struct TASK *receiver_task;
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

void wq_push(struct work *w)
{
	list_pushback(&wq.list, &w->link);
	if(wq.receiver_task != NULL) {
		task_run(wq.receiver_task);
	}
	return;
}

struct work* wq_pop()
{
	return (struct work*)list_popfront(&wq.list);
}

int wq_empty()
{
	return list_empty(&wq.list);
}
