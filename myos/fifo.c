#include "bootpack.h"


#define FLAGS_OVERRUN 1
void fifo32_init(struct FIFO32 *fifo, int size, int *buf,struct TASK *task)
/* FIFO初始化 */
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size; /* 空 */
	fifo->flags = 0;
	fifo->p = 0; /* 写入位置 */
	fifo->q = 0; /* 读取位置 */
	fifo->task = task;
	return;
}

int fifo32_put(struct FIFO32 *fifo, int data)
/* 给FIFO发送数据并储存在FIFO */
{
	if (fifo->free == 0) {
		/* 溢出 */
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->p] = data;
	fifo->p++;
	if (fifo->p == fifo->size) {
		fifo->p = 0;
	}
	fifo->free--;
	if (fifo->task->flags != 2){
		task_run(fifo->task,0);//唤醒
	}
	return 0;
}

int fifo32_get(struct FIFO32 *fifo)

{
	int data;
	if (fifo->free == fifo->size) {
		/* 空队列 */
		return -1;
	}
	data = fifo->buf[fifo->q];
	fifo->q++;
	if (fifo->q == fifo->size) {
		fifo->q = 0;
	}
	fifo->free++;
	return data;
}

int fifo32_status(struct FIFO32 *fifo)
/*  */
{
	return fifo->size - fifo->free;
}
