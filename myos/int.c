/* 妱傝崬傒娭學 */

#include "bootpack.h"





struct FIFO32 *keyfifo;
int keydata0;

struct FIFO32 *mousefifo;
int mousedata0;



void init_pic(void)
/* PIC偺弶婜壔 */
{
	io_out8(PIC0_IMR,  0xff  ); /* 慡偰偺妱傝崬傒傪庴偗晅偗側偄 */
	io_out8(PIC1_IMR,  0xff  ); /* 慡偰偺妱傝崬傒傪庴偗晅偗側偄 */

	io_out8(PIC0_ICW1, 0x11  ); /* 僄僢僕僩儕僈儌乕僪 */
	io_out8(PIC0_ICW2, 0x20  ); /* IRQ0-7偼丄INT20-27偱庴偗傞 */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1偼IRQ2偵偰愙懕 */
	io_out8(PIC0_ICW4, 0x01  ); /* 僲儞僶僢僼傽儌乕僪 */

	io_out8(PIC1_ICW1, 0x11  ); /* 僄僢僕僩儕僈儌乕僪 */
	io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15偼丄INT28-2f偱庴偗傞 */
	io_out8(PIC1_ICW3, 2     ); /* PIC1偼IRQ2偵偰愙懕 */
	io_out8(PIC1_ICW4, 0x01  ); /* 僲儞僶僢僼傽儌乕僪 */

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 PIC1埲奜偼慡偰嬛巭 */
	io_out8(PIC1_IMR,  0xff  ); /* 11111111 慡偰偺妱傝崬傒傪庴偗晅偗側偄 */

	return;
}

void inthandler21(int *esp)
/* 键盘 */
{
	//struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	//boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
	//putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 21 (IRQ-1) : PS/2 keyboard");
	
	unsigned char keydata;
	io_out8(PIC0_OCW2, 0x61);//IRQ-01 已受理完毕
	keydata=io_in8(PORT_KEYDAT);
	fifo32_put(keyfifo,keydata+keydata0);
	return ;
}

void inthandler2c(int *esp)
/*鼠标 */
{
	unsigned char mousedata;
	io_out8(PIC1_OCW2, 0x64);//pic1 IRQ-12 已受理完毕
	io_out8(PIC0_OCW2, 0x62);//pic0 IRQ-02 已受理完毕
	mousedata=io_in8(PORT_KEYDAT);
	fifo32_put(mousefifo,mousedata+mousedata0);
	return;
	
}

void inthandler27(int *esp)
//系统启动要调用7号中断									*/
{
	io_out8(PIC0_OCW2, 0x67); /* IRQ-07庴晅姰椆傪PIC偵捠抦(7-1嶲徠) */
	return;
}
void wait_KBC_sendready(void)
{
	/* 等待控制电路准备完毕 */
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}
void init_keyboard(struct FIFO32 *fifo, int data0)
{
	//将FIFO缓冲区保存到全局变量里
	keyfifo = fifo;
	keydata0 = data0;
	/* 初始化键盘控制电路 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}


void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec)
{
	//将FIFO缓冲区保存到全局变量里
	mousefifo = fifo;
	mousedata0 = data0;
	/* 鼠标有效 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	/* 发送ACK(0xfa)进行确认 */
	mdec->phase = 0; /* 等待鼠标的oxfa阶段 */
	return;
}

