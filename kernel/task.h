#ifndef _TASK_H_
#define _TASK_H_

#define TASK_GDT 3
#define NULL 0

struct TSS32 {
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
};

struct TASK {
	struct TASK *next;
	int sel;
	struct TSS32 tss;
};

struct TASKCTL {
	int next_task_sel;
	struct TASK *running_first;
	struct TASK *running_last;
};

struct TASK *task_init();
struct TASK *task_alloc();
void task_run(struct TASK *new_task);
void task_switch();
void task_show();

#endif
