
#include "bootpack.h"

//内存检查时将缓存设为OFF
unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 1;
	unsigned int eflg, cr0, i;

	/* 确认是386还是486以上的 */
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT; /* AC-bit = 1 */
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	if ((eflg & EFLAGS_AC_BIT) != 0) { /* 如果是386，就是设定AC=1，AC的值还会自动回到0 */
		flg486 = 1;
	}
	eflg &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */
	io_store_eflags(eflg);
	

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; /* 禁止缓存 */
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; /* 允许缓存 */
		store_cr0(cr0);
	}

	return i;
}

void memman_init(struct MEMMAN *man)
{
	man->frees = 0;			/* 可用信息数目 */
	man->maxfrees = 0;		/* 用于观察可用状态 */
	man->lostsize = 0;		/* 释放失败的内存大小总和 */
	man->losts = 0;			/* 释放失败次数 */
	return;
}

unsigned int memman_total(struct MEMMAN *man)
/* 报告空余内存大小合计 */
{
	unsigned int i, t = 0;
	for (i = 0; i < man->frees; i++) {
		t += man->free[i].size;
	}
	return t;
}
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
	unsigned int i=0,addr=0;
	for(;i<man->frees;i++){
		if(size<man->free[i].size){/*有可用内存*/
		
			addr=man->free[i].addr;
			man->free[i].addr+=size;
			man->free[i].size-=size;
			
			if(man->free[i].size == 0){/*内存块减1*/
			
					man->frees--;
					for(;i<man->frees;i++)
						man->free[i]=man->free[i+1];
			}
			break;
		}
	}
	return addr;
}
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	unsigned int i=0,j;
	/*addr 在哪2个内存块(i-1  addr  i )之间*/
	for(;i<man->frees;i++){
		if(addr<man->free[i].addr){
			break;
		}
	}
	/*此块和i-1 可以结合*/
	
	if((man->free[i-1].addr+man->free[i-1].size == addr) && i>0){
		man->free[i-1].size+=size;
		/*如果此块还可以和i块结合 则 i-1 addr i 合成一个快*/
		if((addr+size == man->free[i].addr) && i < man->frees) {
			man->free[i-1].size+=man->free[i].size;
	
			man->frees--;
			for(;i<man->frees;i++)
				man->free[i]=man->free[i+1];
		}
		return 0; /*成功完成*/
	}
	if (i < man->frees) {
						/* 不能和前面结合 */
		if (addr + size == man->free[i].addr) {
			/* 与后面结合 */
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0;  /*成功完成*/
		}
	}
/*如果和两块都不结合就增加个块*/
	if (man->frees < MEMMAN_FREES) {
		/*  */
		for (j = man->frees; j > i; j--) {
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		if (man->maxfrees < man->frees) {
			man->maxfrees = man->frees; /* 更新 free最大值*/
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0; /* 成功 */
	}
		/* 屻傠偵偢傜偣側偐偭偨 */
	man->losts++;
	man->lostsize += size;
	return -1; /* 失败 */
}
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
	unsigned int a;
	size = (size + 0xfff) & 0xfffff000;
	a = memman_alloc(man, size);
	return a;
}

int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i;
	size = (size + 0xfff) & 0xfffff000;
	i = memman_free(man, addr, size);
	return i;
}
