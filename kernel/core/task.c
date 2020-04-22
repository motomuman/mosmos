#include "task.h"
#include "nasmfunc.h"
#include "dsctbl.h"
#include "memory.h"
#include "print.h"
#include "lib.h"

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
	list_init(&taskctl.list);

	struct TASK *initial_task = &taskctl.tasks[taskctl.next_task_id];
	taskctl.next_task_id++;
	task_run(initial_task);
	return initial_task;
}

struct TASK *task_alloc(void (*func)())
{
    struct TASK *task = &taskctl.tasks[taskctl.next_task_id];
    uint64_t stack = mem_alloc(STACK_LENGTH * 8, "stack");
    task->rsp = (uint64_t)(stack + STACK_LENGTH);
    task->rip = (uint64_t)func;
    task->flag = TASK_INITIALIZED;
    task->task_id = taskctl.next_task_id;
    taskctl.next_task_id++;

    // Initialize stack
    struct stackframe64 *sf = (struct stackframe64 *)(task->rsp - sizeof(struct stackframe64));
    memset(sf, 0, sizeof(struct stackframe64));

    sf->sp = task->rsp - STACK_LENGTH/2;
    sf->ip = task->rip;
    sf->cs = 24 + 3;
    sf->ss = 32 + 3;
    sf->fs = 32 + 3;
    sf->gs = 32 + 3;
    sf->flags = 0x202;
    task->rsp = task->rsp - sizeof(struct stackframe64);

    return task;
}

struct TASK *current_task()
{
	return (struct TASK *) list_head(&taskctl.list);
}

void task_run(struct TASK *new_task) {
	// task is already running
	if(new_task->flag == TASK_RUNNING) {
		return;
	}
	new_task->flag = TASK_RUNNING;
	list_pushback(&taskctl.list, &new_task->link);
}

void task_sleep()
{
       // can not sleep if no running task or only one task is running
       struct TASK *current_task = (struct TASK *) list_head(&taskctl.list);
       //printstr_log("task_sleep: ");
       //printnum_log(current_task->task_id);
       //printstr_log("\n");
       //task_show();
       if(current_task == NULL || list_next(&current_task->link) == NULL) {
               return;
       }
       current_task->flag = TASK_WAITING;
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
	struct TASK *current_task = (struct TASK *) list_head(&taskctl.list);
	struct TASK *next_task = (struct TASK *)list_next(&current_task->link);
	if(next_task != NULL) {
		list_popfront(&taskctl.list);
		if (current_task->flag != TASK_WAITING) {
			list_pushback(&taskctl.list, &current_task->link);
		}
		printstr_log("shedule:");
		printnum_log(current_task->task_id);
		printstr_log(" -> ");
		printnum_log(next_task->task_id);
		printstr_log("\n");

		rsp[0] = &current_task->rsp;
		rsp[1] = &next_task->rsp;
	}
	return rsp;
}

void task_show()
{
	struct TASK *task;
	for(task = (struct TASK *) list_head(&taskctl.list); task != NULL; task = (struct TASK *)list_next(&task->link)) {
		printnum_log(task->task_id);
		printstr_log(" -> ");
	}
	printstr_log("\n");
}


