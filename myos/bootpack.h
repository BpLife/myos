/* asmhead.nas */
struct BOOTINFO { /* 0x0ff0-0x0fff */
	char cyls; /* 僽乕僩僙僋僞偼偳偙傑偱僨傿僗僋傪撉傫偩偺偐 */
	char leds; /* 僽乕僩帪偺僉乕儃乕僪偺LED偺忬懺 */
	char vmode; /* 價僨僆儌乕僪  壗價僢僩僇儔乕偐 */
	char reserve;
	short scrnx, scrny; /* 夋柺夝憸搙 */
	char *vram;
};
#define ADR_BOOTINFO	0x00000ff0

/* naskfunc.nas */
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);


/* graphic.c */
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen8(char *vram, int x, int y);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int vxsize, int pxsize,
int pysize, int px0, int py0, char *buf, int bxsize);
#define COL8_000000		0 	//黑
#define COL8_FF0000		1	//亮红
#define COL8_00FF00		2	//亮绿
#define COL8_FFFF00		3	//亮黄
#define COL8_0000FF		4	//亮蓝
#define COL8_FF00FF		5	//亮紫
#define COL8_00FFFF		6	//浅亮蓝
#define COL8_FFFFFF		7	//白
#define COL8_C6C6C6		8	//亮灰
#define COL8_840000		9	//暗红
#define COL8_008400		10	//暗绿
#define COL8_848400		11	//暗黄
#define COL8_000084		12	//暗蓝
#define COL8_840084		13	//暗紫
#define COL8_008484		14	//浅暗蓝
#define COL8_848484		15	//暗灰


/* dsctbl.c */
struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};
struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};
void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e


/* int.c */
void init_pic(void);
void inthandler21(int *esp);
void inthandler27(int *esp);
void inthandler2c(int *esp);


#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

/*为实现鼠标中断分两步。
1.完成键盘控制电路的初始（鼠标控制电路在键盘控制电路里面）化正常工作。
2.一个是鼠标本身*/

#define PORT_KEYDAT				0x0060
#define PORT_KEYSTA				0x0064
#define PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4
//鼠标移动
struct MOUSE_DEC
{
	unsigned char buff[3],phase;
	int x,y,btn;
};

void enable_mouse(struct MOUSE_DEC*);
void init_keyboard(void);


/**************************************************/
//fifo.c
/*FIFO 结构*/
struct FIFO 
{
	unsigned char *buff;
	int head,tail,size,free;
};
int fifo8_init(struct FIFO *fifo8,int buff);
int fifo8_put(struct FIFO* fifo8,unsigned char c);
unsigned char  fifo8_get(struct FIFO *fifo8);



//memory.c
#define MEMMAN_FREES		4090	/* 大约是32KB */
#define MEMMAN_ADDR			0x003c0000

struct FREEINFO {	/* 可用信息 */
	unsigned int addr, size;
};

struct MEMMAN {		/* 内存管理 */
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};


#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000
unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);


//sheet.c
#define MAX_SHEETS		256//再次设定256个图层
struct SHEET {
	unsigned char *buf;
	int bxsize, bysize, vx0, vy0, col_inv/*透明色*/, height, flags/*是否正在被使用*/;
	struct SHTCTL *ctl;
};

struct SHTCTL {
	unsigned char *vram;
	int xsize, ysize, top/*最上层图层的高度*/;
	struct SHEET *sheets[MAX_SHEETS];
	struct SHEET sheets0[MAX_SHEETS];
};

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);

void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
void sheet_updown(struct SHEET *sht, int height);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHEET *sht);
void sheet_slideSuper( struct SHEET *sht, int vx0, int vy0);
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1);

/*void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height);
void sheet_refresh(struct SHTCTL *ctl, struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHTCTL *ctl, struct SHEET *sht);
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1);
void sheet_slideSuper(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0);
*/