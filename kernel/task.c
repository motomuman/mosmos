#include "task.h"
#include "nasmfunc.h"
#include "dsctbl.h"
#include "memory.h"
#include "print.h"

struct TASKCTL taskctl;


struct TASK *task_init()
{
	taskctl.next_task_sel = TASK_GDT;
	taskctl.running_first = NULL;
	taskctl.running_last = NULL;

	struct TASK *initial_task = task_alloc();
	task_run(initial_task);
	load_tr(initial_task->sel * 8);
	return initial_task;
}

struct TASK *task_alloc()
{
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	struct TASK *new_task = (struct TASK*) mem_alloc(sizeof(struct TASK));
	new_task->next = NULL;
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
	if(taskctl.running_first == NULL) {
		taskctl.running_first = new_task;
		taskctl.running_last = new_task;
		return;
	}
	taskctl.running_last->next = new_task;
	taskctl.running_last = new_task;
}

void task_switch() {
	struct TASK *current_task = taskctl.running_first;
	struct TASK *next_task = current_task->next;
	if(next_task != NULL) {
		taskctl.running_first = next_task;
		taskctl.running_last->next = current_task;
		taskctl.running_last = current_task;
		taskctl.running_last->next = NULL;
		farjmp(0, taskctl.running_first->sel * 8);
	}
}

void task_show()
{
	printstr("task list\n");
	struct TASK *task;
	for(task = taskctl.running_first; task != NULL; task = task->next) {
		printnum(task->sel);
		printstr(" -> ");
	}
	printstr("\n");
}


