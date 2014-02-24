#include <stdio.h>
#include"bootpack.h"

extern struct FIFO keyfifo;
extern struct FIFO mousefifo;

int mouse_decode(struct MOUSE_DEC* mousedec,unsigned char mousedata);
void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;//the boot info saved in asmhead
	char s[40], mcursor[256];//the cursor resource
	int mx, my;//the position of cursor
	unsigned char keybuff[32],mousebuff[128];
	unsigned char keydata;
	unsigned char mousedata;
	struct MOUSE_DEC mousedec;
	//内存管理
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	unsigned int memtotal;
	
	init_gdtidt();
	init_pic();//init the programed interrupt controller.
	io_sti(); // enable interrupt
	
	
	
	
	
	init_palette();
	
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
	
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;
	
	init_mouse_cursor8(mcursor, COL8_008484);//display mouse 
	
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);//copy the cursor resource into vram .
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
	
	fifo8_init(&keyfifo,keybuff);
	fifo8_init(&mousefifo,mousebuff);
	io_out8(PIC0_IMR, 0xf9); /* PIC1  允许 1,2号中断，屏蔽其他(11111001) */
	io_out8(PIC1_IMR, 0xef); /* 允许c号中断，屏蔽其他(11101111) */
	
	
	//鼠标中断
	init_keyboard();
	enable_mouse(&mousedec);
	
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	
	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

	
	for (;;) {
		io_cli();
		if(fifo8_status(&keyfifo) + fifo8_status(&mousefifo)==0)
			io_stihlt();// 开中断然后睡眠
		else{
			if(fifo8_status(&keyfifo)!=0){
				keydata=fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", keydata);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
			}
			else if(fifo8_status(&mousefifo)!=0){
				mousedata=fifo8_get(&mousefifo);
				io_sti();
				if(mouse_decode(&mousedec,mousedata)!=0){
					sprintf(s, "[lcr %4d %4d]", mousedec.x, mousedec.y);
					if ((mousedec.btn & 0x01) != 0) {
						s[1] = 'L';
					}
					if ((mousedec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mousedec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					boxfill8(binfo->vram, binfo->scrnx, COL8_FF0000, 32, 16, 32+8*20, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
					
					/* 鼠标指针的移动 */
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15); /* 隐藏鼠标 */
					mx += mousedec.x;
					my += mousedec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 16) {
						mx = binfo->scrnx - 16;
					}
					if (my > binfo->scrny - 16) {
						my = binfo->scrny - 16;
					}
					sprintf(s, "(%3d, %3d)", mx, my);
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15); /* 隐藏坐标 */
					putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s); /* 显示坐标 */
					putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); /* 描绘鼠标 */
				}
			}
		}
	}
}
int mouse_decode(struct MOUSE_DEC* mousedec,unsigned char mousedata)
{

	int phase=mousedec->phase;
	unsigned char *p=&mousedec->phase;
	if(phase==0){
		if(mousedata==0xfa)
			*p=1;
		return 0;
	}
	else if(phase==1){
		if((mousedata&0xc8)==0x08){
			//如果第一字正确
			mousedec->buff[0]=mousedata;
			*p=2;
		}
		return 0;
	}
	else if(phase==2){
		mousedec->buff[1]=mousedata;
		*p=3;
		return 0;
	}
	else if(phase==3){
		mousedec->buff[2]=mousedata;
		*p=1;
		
		mousedec->btn=mousedec->buff[0]&0x07;//取低3位
		mousedec->x=mousedec->buff[1];
		mousedec->y=mousedec->buff[2];
		
		if ((mousedec->buff[0] & 0x10) != 0) {
			mousedec->x |= 0xffffff00;
		}
		if ((mousedec->buff[0] & 0x20) != 0) {
			mousedec->y |= 0xffffff00;
		}
		mousedec->y = - mousedec->y; /* 鼠标坐标与屏幕坐标相反 */
		return 1;
	}
	return -1;
}




/*
	
*/



