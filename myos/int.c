/* 割り込み関係 */

#include "bootpack.h"





struct FIFO32 *keyfifo;
int keydata0;

struct FIFO32 *mousefifo;
int mousedata0;



void init_pic(void)
/* PICの初期化 */
{
	io_out8(PIC0_IMR,  0xff  ); /* 全ての割り込みを受け付けない */
	io_out8(PIC1_IMR,  0xff  ); /* 全ての割り込みを受け付けない */

	io_out8(PIC0_ICW1, 0x11  ); /* エッジトリガモード */
	io_out8(PIC0_ICW2, 0x20  ); /* IRQ0-7は、INT20-27で受ける */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1はIRQ2にて接続 */
	io_out8(PIC0_ICW4, 0x01  ); /* ノンバッファモード */

	io_out8(PIC1_ICW1, 0x11  ); /* エッジトリガモード */
	io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15は、INT28-2fで受ける */
	io_out8(PIC1_ICW3, 2     ); /* PIC1はIRQ2にて接続 */
	io_out8(PIC1_ICW4, 0x01  ); /* ノンバッファモード */

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 PIC1以外は全て禁止 */
	io_out8(PIC1_IMR,  0xff  ); /* 11111111 全ての割り込みを受け付けない */

	return;
}

void inthandler21(int *esp)
/* ｼ�ﾅﾌ */
{
	
	
	unsigned char keydata;
	io_out8(PIC0_OCW2, 0x61);//IRQ-01 ﾒﾑﾊﾜﾀ�ﾍ�ｱﾏ
	keydata=io_in8(PORT_KEYDAT);
	fifo32_put(keyfifo,keydata+keydata0);
	return ;
}

void inthandler2c(int *esp)
/*ﾊ�ｱ� */
{
	unsigned char mousedata;
	io_out8(PIC1_OCW2, 0x64);//pic1 IRQ-12 ﾒﾑﾊﾜﾀ�ﾍ�ｱﾏ
	io_out8(PIC0_OCW2, 0x62);//pic0 IRQ-02 ﾒﾑﾊﾜﾀ�ﾍ�ｱﾏ
	mousedata=io_in8(PORT_KEYDAT);
	fifo32_put(mousefifo,mousedata+mousedata0);
	return;
	
}

void inthandler27(int *esp)
//ﾏｵﾍｳﾆ�ｶｯﾒｪｵ�ﾓﾃ7ｺﾅﾖﾐｶﾏ									*/
{
	io_out8(PIC0_OCW2, 0x67); /* IRQ-07受付完了をPICに通知(7-1参照) */
	return;
}
void wait_KBC_sendready(void)
{
	/* ｵﾈｴ�ｿﾘﾖﾆｵ酊ｷﾗｼｱｸﾍ�ｱﾏ */
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}
void init_keyboard(struct FIFO32 *fifo, int data0)
{
	//ｽｫFIFOｻｺｳ衂�ｱ｣ｴ豬ｽﾈｫｾﾖｱ菽ｿﾀ�
	keyfifo = fifo;
	keydata0 = data0;
	/* ｳ�ﾊｼｻｯｼ�ﾅﾌｿﾘﾖﾆｵ酊ｷ */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}


void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec)
{
	//ｽｫFIFOｻｺｳ衂�ｱ｣ｴ豬ｽﾈｫｾﾖｱ菽ｿﾀ�
	mousefifo = fifo;
	mousedata0 = data0;
	/* ﾊ�ｱ�ﾓﾐﾐｧ */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	/* ｷ｢ﾋﾍACK(0xfa)ｽ�ﾐﾐﾈｷﾈﾏ */
	mdec->phase = 0; /* ｵﾈｴ�ﾊ�ｱ�ｵﾄoxfaｽﾗｶﾎ */
	return;
}

