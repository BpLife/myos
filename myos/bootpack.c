#include <stdio.h>
#include"bootpack.h"

extern struct FIFO32 keyfifo;
extern struct FIFO32 mousefifo;

int mouse_decode(struct MOUSE_DEC* mousedec,unsigned char mousedata);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
void HariMain(void)
{
	static char keytable[0x54] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
	};
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;//the boot info saved in asmhead
	char s[40], mcursor[256];//the cursor resource
	int i,mx, my;//the position of cursor
	
	struct FIFO32 fifo;
	int fifobuf[128];
	
	//timer
	struct TIMER *timer, *timer2, *timer3;
	
	//
	int cursor_x, cursor_c;
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;//白色
	
	

	
	
	unsigned char keydata;
	unsigned char mousedata;
	unsigned char timedata;
	struct MOUSE_DEC mdec;
	//内存管理结构地址
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	unsigned int memtotal;
	
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse , *sht_win;
	unsigned char *buf_back, buf_mouse[256],*buf_win;
	unsigned int count=0;
	
	init_gdtidt();
	init_pic();//init the programed interrupt controller.
	io_sti(); // enable interrupt
	init_pit();//初始化计时器;
	
	
	//初始化队列
	fifo32_init(&fifo, 128, fifobuf);
	
	//鼠标中断
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);
	
	io_out8(PIC0_IMR, 0xf8); /* PIC1  允许 0(timer),1,2号中断，屏蔽其他(11111000) */
	io_out8(PIC1_IMR, 0xef); /* 允许c号中断，屏蔽其他(11101111) */
	
	
	
	
	
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
	sheet_slide( sht_back, 0, 0);
	
	/*显示鼠标图标*/
	mx = (binfo->scrnx - 16) / 2; /* 中间位置 */
	my = (binfo->scrny - 28 - 16) / 2;
	sheet_slide( sht_mouse, mx, my);
	
	sht_win   = sheet_alloc(shtctl);
	buf_win   = (unsigned char *) memman_alloc_4k(memman, 160 * 68);
	sheet_setbuf(sht_win, buf_win, 160, 68, -1); /* 没有透明色 */
	make_window8(buf_win, 160, 68, "counter");
	make_textbox8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);
	sheet_slide(sht_win, 80, 72);
	
	
	
	/*设置图层高度，并显示*/
	sheet_updown(sht_back,  0);
	sheet_updown(sht_win,   1);
	sheet_updown(sht_mouse, 2);
	
	//timer 
	timer = timer_alloc();
	timer_init(timer, &fifo, 10);
	timer_settime(timer, 1000);
	
	timer2 = timer_alloc();
	timer_init(timer2, &fifo, 3);
	timer_settime(timer2, 300);
	
	timer3 = timer_alloc();
	timer_init(timer3, &fifo, 1);
	timer_settime(timer3, 50);
	
	
	
	
	
	
	
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
	
	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);
	
	
	
	for (;;) {
		count++;
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_sti();
		} 
		else{
			i = fifo32_get(&fifo);
			io_sti();
			if (256 <= i && i <= 511) { /*键盘数据 */
				sprintf(s, "%02X", i - 256);
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
				if (i < 256 + 0x54) {
					if (keytable[i - 256] != 0 && cursor_x < 144) {
						s[0] = keytable[i - 256];
						s[1] = 0;
						putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
						cursor_x += 8;
						
					}
				}
				if (i == 256 + 0x0e && cursor_x > 8) { /* 退格键 */
					/* 用空格键把光标消去后，后移1次光标 */
					putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
					cursor_x -= 8;
				}
				//光标显示
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
					
			} else if (512 <= i && i <= 767) { /* 鼠标数据 */
				if (mouse_decode(&mdec, i - 512) != 0) {
					/* 已收集了3字节数据，所以显示出来 */
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) {
						s[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					putfonts8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);
					/* 鼠标指针移动 */
					mx += mdec.x;
					my += mdec.y;
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
					putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
					sheet_slide(sht_mouse, mx, my);
					if ((mdec.btn & 0x01) != 0) {
						/* 始终定位鼠标的左上角 */
						sheet_slide(sht_win, mx - 80, my - 8);
					}
				}
			}
			 else if (i == 10) { /* 10s */
				putfonts8_asc_sht(sht_back, 0, 64, COL8_FFFFFF, COL8_008484, "10[sec]", 7);
				sprintf(s, "%010d", count);
				
			} else if (i == 3) { /* 3s */
				putfonts8_asc_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_008484, "3[sec]", 6);
				count = 0; /* 开始测试 */
			} else if (i <= 1) { /* 光标定时器 */
				if (i != 0) {
					timer_init(timer3, &fifo, 0); /* 下面设定0 */
					cursor_c = COL8_000000;
				} else {
					timer_init(timer3, &fifo, 1); /* 下面设定1 */
					cursor_c = COL8_FFFFFF;
				}
				timer_settime(timer3, 50);
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			}
		}
		
	}
}

void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)
{
	boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
	putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
	return;
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
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c)
{
	int x1 = x0 + sx, y1 = y0 + sy;
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, c,           x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}



