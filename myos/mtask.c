#include "bootpack.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;

//现在正在运行的任务已经变成了一个任务
struct TASK *task_init(struct MEMMAN *memman)
{
	int i;
	struct TASK *task;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	taskctl = (struct TASKCTL *) memman_alloc_4k(memman, sizeof (struct TASKCTL));
	for (i = 0; i < MAX_TASKS; i++) {
		taskctl->tasks0[i].flags = 0;
		taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->tasks0[i].tss, AR_TSS32);
	}
	task = task_alloc();
	task->flags = 2; /* 活动标志 */
	task->priority = 2;
	taskctl->running = 1;//0.02秒
	taskctl->now = 0;
	taskctl->tasks[0] = task;
	load_tr(task->sel);
	task_timer = timer_alloc();
	timer_settime(task_timer, 2);
	return task;
}
struct TASK *task_alloc(void)
{
	int i;
	struct TASK *task;
	for (i = 0; i < MAX_TASKS; i++) {
		if (taskctl->tasks0[i].flags == 0) {
			task = &taskctl->tasks0[i];
			task->flags = 1; /* 正在使用标志 */
			task->tss.eflags = 0x00000202; /* IF = 1; */
			task->tss.eax = 0; /* 这里填充为0 */
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.ldtr = 0;
			task->tss.iomap = 0x40000000;
			return task;
		}
	}
	return 0; /* 全部正在使用 */
}
void task_run(struct TASK *task, int priority)
{
	if (priority >0){
		task->priority = priority;
	}
	if (task->flags != 2){ /* 活动中标志 */
		task->flags = 2;
		taskctl->tasks[taskctl->running] = task;
		taskctl->running++;
	}
	return;
}

void task_switch(void)
{
	struct TASK *task;
	taskctl->now++;
	if (taskctl->now == taskctl->running) {
			taskctl->now = 0;
	}
	task = taskctl->tasks[taskctl->now];
	timer_settime(task_timer, task->priority);
	
	if (taskctl->running >= 2) {
		farjmp(0, taskctl->tasks[taskctl->now]->sel);
	}
	return;
}



