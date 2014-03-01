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
	int i=0;
	io_out8(PIT_CTRL,0x34);
	io_out8(PIT_CNT0,0x9c);
	io_out8(PIT_CNT0,0x2e);
	timerctl.count=0;
	for(;i<MAX_TIMER;i++)
		timerctl.timer[i].flag=0;//未使用
	return;
}
struct TIMER * timer_alloc(void){
	int i=0;
	for(;i<MAX_TIMER;i++){
		if(timerctl.timer[i].flag==0){
			timerctl.timer[i].flag=TIMER_FLAGS_ALLOC;
			return &timerctl.timer[i];
		}
	}
	return 0;/*没找到*/
}
void timer_free(struct TIMER * ptimer){
	ptimer->flag=0;
}
void settimer(struct TIMER* timer,unsigned int timeout,struct FIFO8 *fifo,unsigned char data){
	int eflags;
	eflags=io_load_eflags();
	io_cli();//禁止中断,如果设定还没有完全结束IRQ0，中断就进来会引起混乱
	struct TIMER* timer;
	timer=timer_alloc();
	timer->flag=TIMER_FLAGS_USING;
	timer->timeout=timeout+timerctl.count;
	timer->fifo=fifo;
	timer->data=data;
	io_store_eflags(eflags);
	io_sti();
	return;
}
void inthandler20(int*esp){
	int i=0;
	io_out8(PIC0_OCW2,0x60);//把IRQ-00 信号接收完了的信息通知给PiC*/
	timerctl.count++;
	for(i=0;i<MAX_TIMER;i++){
		if(timerctl.timer[i].flag==TIMER_FLAGS_USING){
			if(timerctl.timer[i].timeout<=timerctl.count){
				timerctl.timer[i].flag=TIMER_FLAGS_ALLOC;
				fifo8_put(timerctl.timer[i].fifo,timerctl.timer[i].data);
			}
		}
	}
	return;
}