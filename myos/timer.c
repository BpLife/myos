#include "bootpack.h"
#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

#define TIMER_FLAGS_ALLOC 1 /*已配置状态*/
#define TIMER_FLAGS_USING 2 	/*定时器运行中*/

struct TIMERCTL timerctl;

/*struct TIMER{
	unsigned int timeout,flags/*记录各个定时器的状态;
	struct FIFO8* fifo;
	unsigned char data;
};*/

void init_pit(void){

	int i;
	struct TIMER*t;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	
	for (i = 0; i < MAX_TIMER; i++) {
		timerctl.timers0[i].flags = 0; //未使用
	}
	t=timer_alloc();
	t->timeout=0xffffffff;
	t->flags=TIMER_FLAGS_USING;
	t->next_timer=0;
	
	timerctl.t0=t;
	timerctl.next_time = 0xffffffff; /* 因为最初没有正在运行的定时器 */
	
	return;
}
struct TIMER * timer_alloc(void){
	int i=0;
	for(;i<MAX_TIMER;i++){
		if(timerctl.timers0[i].flags == 0){
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timers0[i];
		}
	}
	return 0;/*没找到*/
}
void timer_free(struct TIMER * ptimer){
	ptimer->flags = 0;
}

void timer_init(struct TIMER *timer, struct FIFO32 *fifo, unsigned char data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	int e, i, j;
	
	struct TIMER * t,*s;
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	
	timerctl.using++;
	
	t=timerctl.t0;
	
	
	if (timer->timeout <= t->timeout) {
		/* 插入最前面 */
		timerctl.t0 = timer;
		timer->next_timer = t; 
		timerctl.next_time = timer->timeout;
		io_store_eflags(e);
		return;
	}
	/* 寻找插入位置 */
	for (;;) {
		s = t;
		t = t->next_timer;
		if (timer->timeout <= t->timeout) {
			
			s->next_timer = timer; 
			timer->next_timer = t; 
			io_store_eflags(e);
			return;
		}
	}
}


void inthandler20(int*esp){
	int i=0,j;
	char ts = 0;
	io_out8(PIC0_OCW2,0x60);//把IRQ-00 信号接收完了的信息通知给PiC*/
	
	struct TIMER *timer;
	
	timerctl.count++;
	
	if (timerctl.next_time > timerctl.count) {
		return;
	}
	timer = timerctl.t0;
	
	for(;;){
		if(timer->timeout>timerctl.count)
			break;
		timer->flags=TIMER_FLAGS_ALLOC;
		if(timer != task_timer){
			fifo32_put(timer->fifo,timer->data);
		}else{
			ts = 1;
		}
		timer = timer->next_timer;
	}
	
	
	timerctl.t0=timer;
	timerctl.next_time = timer->timeout;
	if (ts != 0){
		task_switch();
	}
	return;
}