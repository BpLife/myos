/* ���荞�݊֌W */

#include "bootpack.h"





struct FIFO32 *keyfifo;
int keydata0;

struct FIFO32 *mousefifo;
int mousedata0;



void init_pic(void)
/* PIC�̏����� */
{
	io_out8(PIC0_IMR,  0xff  ); /* �S�Ă̊��荞�݂��󂯕t���Ȃ� */
	io_out8(PIC1_IMR,  0xff  ); /* �S�Ă̊��荞�݂��󂯕t���Ȃ� */

	io_out8(PIC0_ICW1, 0x11  ); /* �G�b�W�g���K���[�h */
	io_out8(PIC0_ICW2, 0x20  ); /* IRQ0-7�́AINT20-27�Ŏ󂯂� */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1��IRQ2�ɂĐڑ� */
	io_out8(PIC0_ICW4, 0x01  ); /* �m���o�b�t�@���[�h */

	io_out8(PIC1_ICW1, 0x11  ); /* �G�b�W�g���K���[�h */
	io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15�́AINT28-2f�Ŏ󂯂� */
	io_out8(PIC1_ICW3, 2     ); /* PIC1��IRQ2�ɂĐڑ� */
	io_out8(PIC1_ICW4, 0x01  ); /* �m���o�b�t�@���[�h */

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 PIC1�ȊO�͑S�ċ֎~ */
	io_out8(PIC1_IMR,  0xff  ); /* 11111111 �S�Ă̊��荞�݂��󂯕t���Ȃ� */

	return;
}

void inthandler21(int *esp)
/* ���� */
{
	//struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	//boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
	//putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 21 (IRQ-1) : PS/2 keyboard");
	
	unsigned char keydata;
	io_out8(PIC0_OCW2, 0x61);//IRQ-01 ���������
	keydata=io_in8(PORT_KEYDAT);
	fifo32_put(keyfifo,keydata+keydata0);
	return ;
}

void inthandler2c(int *esp)
/*��� */
{
	unsigned char mousedata;
	io_out8(PIC1_OCW2, 0x64);//pic1 IRQ-12 ���������
	io_out8(PIC0_OCW2, 0x62);//pic0 IRQ-02 ���������
	mousedata=io_in8(PORT_KEYDAT);
	fifo32_put(mousefifo,mousedata+mousedata0);
	return;
	
}

void inthandler27(int *esp)
//ϵͳ����Ҫ����7���ж�									*/
{
	io_out8(PIC0_OCW2, 0x67); /* IRQ-07��t������PIC�ɒʒm(7-1�Q��) */
	return;
}
void wait_KBC_sendready(void)
{
	/* �ȴ����Ƶ�·׼����� */
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}
void init_keyboard(struct FIFO32 *fifo, int data0)
{
	//��FIFO���������浽ȫ�ֱ�����
	keyfifo = fifo;
	keydata0 = data0;
	/* ��ʼ�����̿��Ƶ�· */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}


void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec)
{
	//��FIFO���������浽ȫ�ֱ�����
	mousefifo = fifo;
	mousedata0 = data0;
	/* �����Ч */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	/* ����ACK(0xfa)����ȷ�� */
	mdec->phase = 0; /* �ȴ�����oxfa�׶� */
	return;
}

