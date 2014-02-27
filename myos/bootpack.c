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
	//内存管理结构地址
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	unsigned int memtotal;
	
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse , *sht_win;
	unsigned char *buf_back, buf_mouse[256],*buf_win;
	
	
	init_gdtidt();
	init_pic();//init the programed interrupt controller.
	io_sti(); // enable interrupt
	fifo8_init(&keyfifo,keybuff);
	fifo8_init(&mousefifo,mousebuff);
	io_out8(PIC0_IMR, 0xf9); /* PIC1  允许 1,2号中断，屏蔽其他(11111001) */
	io_out8(PIC1_IMR, 0xef); /* 允许c号中断，屏蔽其他(11101111) */
	
	
	//鼠标中断
	init_keyboard();
	enable_mouse(&mousedec);
	
	
	//内存管理初始化
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	
	//调色板
	init_palette();
	//对图层管理结构体的初始化
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	//申请并使用两个图层
	sht_back  = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	//桌面图层缓冲区
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	
	//图层缓冲区的设置
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); /* 没有透明色 */
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 17);/*背景色号17*/
	/*桌面先显示到内存中*/
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);
	/*鼠标显示到内存中*/
	init_mouse_cursor8(buf_mouse, 17);
	/*显示背景色*/
	sheet_slideSuper( sht_back, 0, 0);
	
	/*显示鼠标图标*/
	mx = (binfo->scrnx - 16) / 2; /* 中间位置 */
	my = (binfo->scrny - 28 - 16) / 2;
	sheet_slideSuper( sht_mouse, mx, my);
	
	sht_win   = sheet_alloc(shtctl);
	buf_win   = (unsigned char *) memman_alloc_4k(memman, 160 * 68);
	sheet_setbuf(sht_win, buf_win, 160, 68, -1); /* 没有透明色 */
	make_window8(buf_win, 160, 68, "window");
	sheet_slideSuper(sht_win, 80, 72);

	
	
	/*设置图层高度，并显示*/
	sheet_updown(sht_back,  0);
	sheet_updown(sht_win,   1);
	sheet_updown(sht_mouse, 2);
	
	
	
	
	
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
	
	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
	sheet_refresh( sht_back, 0, 0, binfo->scrnx, 48);
	
	for (;;) {
		io_cli();
		if(fifo8_status(&keyfifo) + fifo8_status(&mousefifo)==0)
			io_stihlt();// 开中断然后睡眠
		else{
			if(fifo8_status(&keyfifo)!=0){
				keydata=fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", keydata);
				boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
				putfonts8_asc(buf_back, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
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
					boxfill8(buf_back, binfo->scrnx, COL8_FF0000, 32, 16, 32+8*20, 31);
					putfonts8_asc(buf_back, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
					sheet_refresh(sht_back, 32, 16, 32+8*20, 31);
					/* 鼠标指针的移动 */
					
					mx += mousedec.x;
					my += mousedec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 1) {
						mx = binfo->scrnx - 1;
					}
					if (my > binfo->scrny - 1) {
						my = binfo->scrny - 1;
					}
					sprintf(s, "(%3d, %3d)", mx, my);
					
					boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 0, 79, 15); /* 隐藏坐标 */
					putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s); /* 显示坐标 */
					sheet_refresh( sht_back, 0, 0, 80, 16);
					//sheet_slide(shtctl, sht_mouse, mx, my); /* 描绘鼠标 */
					sheet_slideSuper(sht_mouse, mx, my); /* 快速描绘鼠标 */
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

void make_window8(unsigned char *buf, int xsize, int ysize, char *title)
{
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	char c;
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_000084, 3,         3,         xsize - 4, 20       );
	boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
	putfonts8_asc(buf, xsize, 24, 4, COL8_FFFFFF, title);
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COL8_000000;
			} else if (c == '$') {
				c = COL8_848484;
			} else if (c == 'Q') {
				c = COL8_C6C6C6;
			} else {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}


/*
	
*/



