; haribote-os boot asm
; TAB=4
[INSTRSET "i486p"]

VBEMODE	EQU		0x101			; 


;	0x101 :  640 x  480 x 8bit分辨率
;	0x103 :  800 x  600 x 8bit分辨率
;	0x105 : 1024 x  768 x 8bit分辨率
;	0x107 : 1280 x 1024 x 8bit分辨率

BOTPAK	EQU		0x00280000		; bootpack内存地址
DSKCAC	EQU		0x00100000		; 
DSKCAC0	EQU		0x00008000		; 僨傿僗僋僉儍僢僔儏偺応強乮儕傾儖儌乕僪乯

; BOOT_INFO信息
CYLS	EQU		0x0ff0			; 
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			; 
SCRNX	EQU		0x0ff4			; 夝憸搙偺X
SCRNY	EQU		0x0ff6			; 夝憸搙偺Y
VRAM	EQU		0x0ff8			; 僌儔僼傿僢僋僶僢僼傽偺奐巒斣抧

		ORG		0xc200			; 偙偺僾儘僌儔儉偑偳偙偵撉傒崬傑傟傞偺偐

; 设定画面模式,VESA BIOS extension 制作专业的BIOS，以达到不同显卡的兼容。利用它可以使用显卡的高分辨率。
;ax=4f02，bx=画面模式



;中断10号的4Fxx下的函数
　;AX=4F00 -获取VESA结构信息
　　; 01 -获取某种显示模式的属性信息
　　 ;02 -设置显示模式
　　 ;03 -获取当前的模式
　　 ;05 -页切换(用于对显示内存直接操作画点,线)
　　;其他的入口,出口参数可参看中断手册


; 确认vbe是否存在
		MOV		AX,0x9000
		MOV		ES,AX
		MOV		DI,0
		MOV		AX,0x4f00;获取显示模式
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn320

; 检查VBE的版本

		MOV		AX,[ES:DI+4]
		CMP		AX,0x0200
		JB		scrn320			; if (AX < 0x0200) goto scrn320

; 获取画面模式信息

		MOV		CX,VBEMODE
		MOV		AX,0x4f01
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn320

; 画面模式的确认

		CMP		BYTE [ES:DI+0x19],8 ;颜色数必须为8
		JNE		scrn320
		CMP		BYTE [ES:DI+0x1b],4 ;颜色的指定方法
		JNE		scrn320
		MOV		AX,[ES:DI+0x00] ;模式属性
		AND		AX,0x0080
		JZ		scrn320			; bit7不是就不好办

; 画面模式的切换

		MOV		BX,VBEMODE+0x4000;画面模式值+00x4000
		MOV		AX,0x4f02
		INT		0x10
		MOV		BYTE [VMODE],8	; 记下画面模式
		MOV		AX,[ES:DI+0x12]
		MOV		[SCRNX],AX
		MOV		AX,[ES:DI+0x14]
		MOV		[SCRNY],AX
		MOV		EAX,[ES:DI+0x28]
		MOV		[VRAM],EAX
		JMP		keystatus

scrn320:
		MOV		AL,0x13			; VGA图，320x200x8bit彩色
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8	; 记下画面模式
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000

keystatus:
		MOV		AH,0x02
		INT		0x16 			; keyboard BIOS
		MOV		[LEDS],AL

;禁止pic0，pic1,所有中断

		MOV		AL,0xff
		OUT		0x21,AL
		NOP						
		OUT		0xa1,AL

		CLI						

; 为了让CPU能够访问1MB内存空间A20GATE
;从理论上讲，打开A20 Gate的方法是通过设置8042芯片输出端口（64h）的2nd-bit，
;但事实上，当你向8042芯片输出端口进行写操作的时候，
;在键盘缓冲区中，或许还有别的数据尚未处理，因此你必须首先处理这些数据。
		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL
		CALL	waitkbdout

; 切换到保护模式

[INSTRSET "i486p"]				; 使用486指令

		LGDT	[GDTR0]			; 设定临时GDT
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; bit31禁止分页
		OR		EAX,0x00000001	; bit0 为1 切换到保护模式
		MOV		CR0,EAX
		JMP		pipelineflush
pipelineflush:
		MOV		AX,1*8			;  可读写的段
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; bootpack传送

		MOV		ESI,bootpack	; 传送源
		MOV		EDI,BOTPAK		; 传送目的地0x00280000
		MOV		ECX,512*1024/4  ;大小为512KB
		CALL	memcpy

; 

; 首先从启动扇区开始

		MOV		ESI,0x7c00		; 源
		MOV		EDI,DSKCAC		; 目的地 1M 0x10000
		MOV		ECX,512/4
		CALL	memcpy

; 所有剩下的

		MOV		ESI,DSKCAC0+512	; 源
		MOV		EDI,DSKCAC+512	; 目的 0x100200
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; 从柱面数变换成字节数/4
		SUB		ECX,512/4		; 减去ipl
		CALL	memcpy

; asmhead 完成的工作至此已完成
;	下面有bootpack完成

; bootpack启动

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; 没有要传送的东西时
		MOV		ESI,[EBX+20]	; 传送源
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; 传送目的地
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; 初始堆栈
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		; 清空数据接收缓冲区的垃圾数据
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; 
		RET


		ALIGNB	16
GDT0:
		RESB	8				; NULL selector
		DW		0xffff,0x0000,0x9200,0x00cf	; 可读写的段 32bit
		DW		0xffff,0x0000,0x9a28,0x0047	; 可以执行的段

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack:
