#ifndef _TASK_H_
#define _TASK_H_

#include "list.h"
#include "types.h"

#define MAX_TASK 100
#define TASK_INITIALIZED 1
#define TASK_RUNNING 2
#define TASK_WAITING 3

#define TASK_PRIORITY_LOW 0
#define TASK_PRIORITY_HIGH 1
#define TASK_PRIORITY_COUNT 2

struct TASK {
	struct list_item link;
	uint64_t rsp;
	uint64_t rip;
	int flag;
	int task_id;
	int priority;
	void* waitcause;
};

struct TASKCTL {
	struct TASK *current_task;
	struct listctl lists[TASK_PRIORITY_COUNT];
	int next_task_id;
	struct TASK tasks[MAX_TASK];
};

struct TASK *task_init();
struct TASK *task_start(void (*func)(), int priority, int is_userland);
void task_wakeup(void *cause);
void task_sleep(void *cause);
uint64_t** schedule();
void task_show();
struct TASK *current_task();

#endif
