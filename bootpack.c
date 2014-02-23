#include <stdio.h>
#include"bootpack.h"

extern struct FIFO keyfifo;
extern struct FIFO mousefifo;
void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;//the boot info saved in asmhead
	char s[40], mcursor[256];//the cursor resource
	int mx, my;//the position of cursor
	unsigned char keybuff[32],mousebuff[128];
	unsigned char keydata;
	unsigned char mousedata;
	
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
	enable_mouse();
	
	
	
	
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
				sprintf(s, "%02X", mousedata);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 47, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
			}
		}
	}
}





/*
	
*/



