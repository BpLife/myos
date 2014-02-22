#include "bootpack.h"
int fifo8_init(struct FIFO *fifo8,int buff)
{
	fifo8->buff=(unsigned char *)buff;
	fifo8->size=32;
	fifo8->free=32;
	fifo8->head=fifo8->tail=0;
	return 1;
}

int fifo8_put(struct FIFO* fifo8,unsigned char c)
{
	if(fifo8->free==0){
			return -1;
	}
	fifo8->free--;
	fifo8->buff[fifo8->tail]=c;
	fifo8->tail++;
	if(fifo8->tail==32)
		fifo8->tail=0;
	return 1;
}
unsigned char  fifo8_get(struct FIFO *fifo8)
{
	unsigned char data;
	if(fifo8->free==fifo8->size)	
		return -1;//Îª¿Õ
	fifo8->free++;
	
    data=fifo8->buff[fifo8->head];
	fifo8->head++;
	if(fifo8->head==32)
		fifo8->head=0;
	return data;
}
/*************/