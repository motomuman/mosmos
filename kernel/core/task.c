#include "task.h"
#include "nasmfunc.h"
#include "dsctbl.h"
#include "memory.h"
#include "print.h"

struct TASKCTL taskctl;

struct TASK *task_init()
{
	taskctl.next_task_sel = TASK_GDT;
	list_init(&taskctl.list);

	struct TASK *initial_task = task_alloc();
	task_run(initial_task);
	load_tr(initial_task->sel * 8);
	return initial_task;
}

struct TASK *task_alloc()
{
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	struct TASK *new_task = &taskctl.tasks[taskctl.next_task_sel];
	new_task->flag = TASK_INITIALIZED;
	new_task->sel = taskctl.next_task_sel;
	taskctl.next_task_sel++;
	set_segmdesc(gdt + new_task->sel, 103, (int) &new_task->tss, AR_TSS32);

	new_task->tss.ldtr = 0;
	new_task->tss.iomap = 0x40000000;
	new_task->tss.eflags = 0x00000202; /* IF = 1; */
	new_task->tss.eax = 0;
	new_task->tss.ecx = 0;
	new_task->tss.edx = 0;
	new_task->tss.ebx = 0;
	new_task->tss.esp  = mem_alloc(1024 * 1024) + (1024 * 1024);
	new_task->tss.ebp = 0;
	new_task->tss.esi = 0;
	new_task->tss.edi = 0;
	new_task->tss.es = 2 * 8;
	new_task->tss.cs = 1 * 8;
	new_task->tss.ss = 2 * 8;
	new_task->tss.ds = 2 * 8;
	new_task->tss.fs = 2 * 8;
	new_task->tss.gs = 2 * 8;
	return new_task;
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
	// no running task or only one task is running
	struct TASK *current_task = (struct TASK *) list_head(&taskctl.list);
	if(current_task == NULL || list_next(&current_task->link) == NULL) {
		return;
	}
	list_popfront(&taskctl.list);
	current_task->flag = TASK_WAITING;

	struct TASK *next_task = (struct TASK *) list_head(&taskctl.list);

	// after remove current task from running queue,
	// Jump to next task
	printstr_log("far jump\n");
	farjmp(0, next_task->sel * 8);
}

void task_switch() {
	struct TASK *current_task = (struct TASK *) list_head(&taskctl.list);
	struct TASK *next_task = (struct TASK *)list_next(&current_task->link);
	if(next_task != NULL) {
		list_popfront(&taskctl.list);
		list_pushback(&taskctl.list, &current_task->link);
		farjmp(0, next_task->sel * 8);
	}
}

void task_show()
{
	printstr_log("task list\n");
	struct TASK *task;
	for(task = (struct TASK *) list_head(&taskctl.list); task != NULL; task = (struct TASK *)list_next(&task->link)) {
		printnum_log(task->sel);
		printstr_log(" -> ");
	}
	printstr_log("\n");
}


