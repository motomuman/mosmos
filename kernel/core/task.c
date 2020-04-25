#include "task.h"
#include "dsctbl.h"
#include "memory.h"
#include "print.h"
#include "lib.h"
#include "asm.h"

#define STACK_LENGTH    0x1000

struct stackframe64 {
    /* Segment registers */
    uint64_t gs;
    uint64_t fs;

    /* Base pointer */
    uint64_t bp;

    /* Index registers */
    uint64_t di;
    uint64_t si;

    /* Generic registers */
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t dx;
    uint64_t cx;
    uint64_t bx;
    uint64_t ax;

    // Stack for iretq
    uint64_t ip;            /* Instruction pointer */
    uint64_t cs;            /* Code segment */
    uint64_t flags;         /* Flags */
    uint64_t sp;            /* Stack pointer */
    uint64_t ss;            /* Stack segment */
} __attribute__ ((packed));

struct TASKCTL taskctl;

struct TASK *task_init()
{
	taskctl.next_task_id = 0;
	list_init(&taskctl.lists[TASK_PRIORITY_LOW]);
	list_init(&taskctl.lists[TASK_PRIORITY_HIGH]);

	struct TASK *initial_task = &taskctl.tasks[taskctl.next_task_id];
	initial_task->priority = TASK_PRIORITY_HIGH;
	initial_task->flag = TASK_RUNNING;

	taskctl.next_task_id++;
	taskctl.current_task = initial_task;
	return initial_task;
}

struct TASK *task_alloc(void (*func)(), int priority, int is_userland)
{
    struct TASK *task = &taskctl.tasks[taskctl.next_task_id];
    uint64_t stack = mem_alloc(STACK_LENGTH * 8, "stack");
    task->rsp = (uint64_t)(stack + STACK_LENGTH);
    task->rip = (uint64_t)func;
    task->flag = TASK_INITIALIZED;
    task->task_id = taskctl.next_task_id;
    task->priority = priority;
    taskctl.next_task_id++;

    // Initialize stack
    struct stackframe64 *sf = (struct stackframe64 *)(task->rsp - sizeof(struct stackframe64));
    memset(sf, 0, sizeof(struct stackframe64));

    sf->sp = task->rsp - STACK_LENGTH/2;
    sf->ip = task->rip;
    if(is_userland) {
	    sf->cs = 24 + 3;
	    sf->ss = 32 + 3;
	    sf->fs = 32 + 3;
	    sf->gs = 32 + 3;
    } else {
	    sf->cs = 8;
	    sf->ss = 16;
	    sf->fs = 16;
	    sf->gs = 16;
    }
    sf->flags = 0x202;
    task->rsp = task->rsp - sizeof(struct stackframe64);

    return task;
}

struct TASK *current_task()
{
	return taskctl.current_task;
}

void task_run(struct TASK *new_task) {
	// task is already running
	if(new_task->flag == TASK_RUNNING) {
		return;
	}
	new_task->flag = TASK_RUNNING;
	list_pushback(&taskctl.lists[new_task->priority], &new_task->link);
}

void task_sleep()
{
       taskctl.current_task->flag = TASK_WAITING;
       task_switch();
}

/*
 * Called from task_switch
 * rsp[0]: *current_rsp
 * rsp[1]: *next_rsp
 */
uint64_t *rsp[2];
uint64_t** schedule() {
	memset(rsp, 0, 2*sizeof(uint64_t));
	struct TASK *current_task = taskctl.current_task;
	struct TASK *next_task = (struct TASK *) list_popfront(&taskctl.lists[TASK_PRIORITY_HIGH]);

	if(next_task == NULL &&
			(current_task->flag == TASK_WAITING || current_task->priority == TASK_PRIORITY_LOW)) {
		next_task = (struct TASK *) list_popfront(&taskctl.lists[TASK_PRIORITY_LOW]);
	}

	if(next_task != NULL) {
		if (current_task->flag != TASK_WAITING) {
			list_pushback(&taskctl.lists[current_task->priority], &current_task->link);
		}
		rsp[0] = &current_task->rsp;
		rsp[1] = &next_task->rsp;

		//printstr_app("shedule:");
		//printnum_app(current_task->task_id);
		//printstr_app(" -> ");
		//printnum_app(next_task->task_id);
		//printstr_app("\n");
		//task_show();

		taskctl.current_task = next_task;
	}
	return rsp;
}

void task_show()
{
	struct TASK *task;
	printstr_app("Current ");
	printnum_app(taskctl.current_task->task_id);
	printstr_app(": High prio");
	for(task = (struct TASK *) list_head(&taskctl.lists[TASK_PRIORITY_HIGH]); task != NULL; task = (struct TASK *)list_next(&task->link)) {
		printnum_app(task->task_id);
		printstr_app(" -> ");
	}

	printstr_app(": Low prio");
	for(task = (struct TASK *) list_head(&taskctl.lists[TASK_PRIORITY_LOW]); task != NULL; task = (struct TASK *)list_next(&task->link)) {
		printnum_app(task->task_id);
		printstr_app(" -> ");
	}
	printstr_app("\n");
}
