#include "bootpack.h"

#define SHEET_USE		1
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
	struct SHTCTL *ctl;
	int i;
	ctl = (struct SHTCTL *) memman_alloc_4k(memman, sizeof (struct SHTCTL));
	if (ctl == 0) {
		goto err;
	}
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1; /* һ��sheet��û�� */
	for (i = 0; i < MAX_SHEETS; i++) {
		ctl->sheets0[i].flags = 0; /* ���Ϊδʹ�� */
	}
err:
	return ctl;
}
struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
	struct SHEET *sht;
	int i=0;
	for(;i<MAX_SHEETS;i++){
		sht=&(ctl->sheets0[i]);
		if(sht->flags==0){
			sht->height=-1;/*��ͼ������*/
			sht->flags=SHEET_USE;/*���ͼ���Ѿ���ʹ��*/
			return sht;
		}	
	}
	return 0;	/*  */
}
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->col_inv = col_inv;/*͸��ɫ*/
	return;
}
void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height)
{
	int h, old = sht->height; /* ����ԭ��ͼ��ĸ߶� */

	/* ����ͼ��ķ�Χ */
	if (height > ctl->top + 1) {
		height = ctl->top + 1;
	}
	if (height < -1) {
		height = -1;
	}
	sht->height = height; /* �����趨�߶� */

	/* sheets[]�Ĵ������� */
	if (old > height) {	/* ����ǰ�ĵ� */
		if (height >= 0) {
			/* ���м�������� */
			for (h = old; h > height; h--) {
				ctl->sheets[h] = ctl->sheets[h - 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		} else {	/* ���� */
			if (ctl->top > old) {
				/* ������Ľ����� */
				for (h = old; h < ctl->top; h++) {
					ctl->sheets[h] = ctl->sheets[h + 1];
					ctl->sheets[h]->height = h;
				}
			}
			ctl->top--; /* �������ͼ��߶ȼ�һ */
		}
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize); /* ����ͼ�����Ϣ���»��ƻ��� */
	} else if (old < height) {	/* �߶ȱ���ǰ�ĸ� */
		if (old >= 0) {
			/* �м����½� */
			for (h = old; h < height; h++) {
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		} else {	/* ������״̬��Ϊ��ʾ״̬ */
			/* ����������������� */
			for (h = ctl->top; h >= height; h--) {
				ctl->sheets[h + 1] = ctl->sheets[h];
				ctl->sheets[h + 1]->height = h + 1;
			}
			ctl->sheets[height] = sht;
			ctl->top++; /* ����ͼ��������һ���������������ͼ���һ */
		}
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize);; /* ����ͼ�����Ϣ���»��ƻ��� */
	}
	return;
}
/*void sheet_refresh(struct SHTCTL *ctl)
{
	int h, bx, by, vx, vy;
	unsigned char *buf, c, *vram = ctl->vram;
	struct SHEET *sht;
	for (h = 0; h <= ctl->top; h++) {
		sht = ctl->sheets[h];
		buf = sht->buf;
		for (by = 0; by < sht->bysize; by++) {
			vy = sht->vy0 + by;
			for (bx = 0; bx < sht->bxsize; bx++) {
				vx = sht->vx0 + bx;
				c = buf[by * sht->bxsize + bx];
				if (c != sht->col_invͼ�㱳��ɫ��ͼ����ɫ��ͬʱ����) {
					vram[vy * ctl->xsize + vx] = c;
				}
			}
		}
	}
	return;
}
*/
/*�Ի��������������ˢ�£����ڿ�����ʾ����*/
void sheet_refresh(struct SHTCTL *ctl, struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
	if (sht->height >= 0) { /* */
		sheet_refreshsub(ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1);
	}
	return;
}
/*void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0)
{
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if (sht->height >= 0) { /* ���������ʾ 
		sheet_refresh(ctl); /* ���»���ͼ�� 
	}
	return;
}
*/
void sheet_slideSuper(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0)
{
	int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if (sht->height >= 0) { /* ���������ʾ������ͼ�����Ϣˢ�»��� */
		sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize);
		sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize);
	}
	return;
}

void sheet_free(struct SHTCTL *ctl, struct SHEET *sht)
{
	if (sht->height >= 0) {
		sheet_updown(ctl, sht, -1); /* ���������ʾ������������ */
	}
	sht->flags = 0; /* ��δʹ�ñ�־�� */
	return;
}
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1)
{
	/*int h, bx, by, vx, vy;
	unsigned char *buf, c, *vram = ctl->vram;
	struct SHEET *sht;
	for (h = 0; h <= ctl->top; h++) {
		sht = ctl->sheets[h];
		buf = sht->buf;
		for (by = 0; by < sht->bysize; by++) {
			vy = sht->vy0 + by;
			for (bx = 0; bx < sht->bxsize; bx++) {
				vx = sht->vx0 + bx;
				if (vx0 <= vx && vx < vx1 && vy0 <= vy && vy < vy1) {/*ֻ�� �ƶ�ͼ���ǰ����
					c = buf[by * sht->bxsize + bx];
					if (c != sht->col_inv) {
						vram[vy * ctl->xsize + vx] = c;
					}
				}
			}
		}
	}
	return;*/
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, c, *vram = ctl->vram;
	struct SHEET *sht;
	for (h = 0; h <= ctl->top; h++) {
		sht = ctl->sheets[h];
		buf = sht->buf;
		/* vx0�`vy1���g���āAbx0�`by1���t�Z���� */
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if (bx0 < 0) { bx0 = 0; }
		if (by0 < 0) { by0 = 0; }
		if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
		if (by1 > sht->bysize) { by1 = sht->bysize; }
		for (by = by0; by < by1; by++) {
			vy = sht->vy0 + by;
			for (bx = bx0; bx < bx1; bx++) {
				vx = sht->vx0 + bx;
				c = buf[by * sht->bxsize + bx];
				if (c != sht->col_inv) {
					vram[vy * ctl->xsize + vx] = c;
				}
			}
		}
	}
	return;
}
