; haribote-os boot asm
; TAB=4
[INSTRSET "i486p"]

VBEMODE	EQU		0x101			; 


;	0x101 :  640 x  480 x 8bit分辨率
;	0x103 :  800 x  600 x 8bit分辨率
;	0x105 : 1024 x  768 x 8bit分辨率
;	0x107 : 1280 x 1024 x 8bit分辨率

BOTPAK	EQU		0x00280000		; bootpack偺儘乕僪愭
DSKCAC	EQU		0x00100000		; 僨傿僗僋僉儍僢僔儏偺応強
DSKCAC0	EQU		0x00008000		; 僨傿僗僋僉儍僢僔儏偺応強乮儕傾儖儌乕僪乯

; BOOT_INFO信息
CYLS	EQU		0x0ff0			; 僽乕僩僙僋僞偑愝掕偡傞
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			; 怓悢偵娭偡傞忣曬丅壗價僢僩僇儔乕偐丠
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

; PIC偑堦愗偺妱傝崬傒傪庴偗晅偗側偄傛偆偵偡傞
;	AT屳姺婡偺巇條偱偼丄PIC偺弶婜壔傪偡傞側傜丄
;	偙偄偮傪CLI慜偵傗偭偰偍偐側偄偲丄偨傑偵僴儞僌傾僢僾偡傞
;	PIC偺弶婜壔偼偁偲偱傗傞

		MOV		AL,0xff
		OUT		0x21,AL
		NOP						; OUT柦椷傪楢懕偝偣傞偲偆傑偔偄偐側偄婡庬偑偁傞傜偟偄偺偱
		OUT		0xa1,AL

		CLI						; 偝傜偵CPU儗儀儖偱傕妱傝崬傒嬛巭

; CPU偐傜1MB埲忋偺儊儌儕偵傾僋僙僗偱偒傞傛偆偵丄A20GATE傪愝掕

		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL
		CALL	waitkbdout

; 僾儘僥僋僩儌乕僪堏峴

[INSTRSET "i486p"]				; 486偺柦椷傑偱巊偄偨偄偲偄偆婰弎

		LGDT	[GDTR0]			; 巄掕GDT傪愝掕
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; bit31傪0偵偡傞乮儁乕僕儞僌嬛巭偺偨傔乯
		OR		EAX,0x00000001	; bit0傪1偵偡傞乮僾儘僥僋僩儌乕僪堏峴偺偨傔乯
		MOV		CR0,EAX
		JMP		pipelineflush
pipelineflush:
		MOV		AX,1*8			;  撉傒彂偒壜擻僙僌儊儞僩32bit
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; bootpack偺揮憲

		MOV		ESI,bootpack	; 揮憲尦
		MOV		EDI,BOTPAK		; 揮憲愭
		MOV		ECX,512*1024/4
		CALL	memcpy

; 偮偄偱偵僨傿僗僋僨乕僞傕杮棃偺埵抲傊揮憲

; 傑偢偼僽乕僩僙僋僞偐傜

		MOV		ESI,0x7c00		; 揮憲尦
		MOV		EDI,DSKCAC		; 揮憲愭
		MOV		ECX,512/4
		CALL	memcpy

; 巆傝慡晹

		MOV		ESI,DSKCAC0+512	; 揮憲尦
		MOV		EDI,DSKCAC+512	; 揮憲愭
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; 僔儕儞僟悢偐傜僶僀僩悢/4偵曄姺
		SUB		ECX,512/4		; IPL偺暘偩偗嵎偟堷偔
		CALL	memcpy

; asmhead偱偟側偗傟偽偄偗側偄偙偲偼慡晹偟廔傢偭偨偺偱丄
;	偁偲偼bootpack偵擟偣傞

; bootpack偺婲摦

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; 揮憲偡傞傋偒傕偺偑側偄
		MOV		ESI,[EBX+20]	; 揮憲尦
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; 揮憲愭
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; 僗僞僢僋弶婜抣
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		; AND偺寢壥偑0偱側偗傟偽waitkbdout傊
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; 堷偒嶼偟偨寢壥偑0偱側偗傟偽memcpy傊
		RET
; memcpy偼傾僪儗僗僒僀僘僾儕僼傿僋僗傪擖傟朰傟側偗傟偽丄僗僩儕儞僌柦椷偱傕彂偗傞

		ALIGNB	16
GDT0:
		RESB	8				; 僰儖僙儗僋僞
		DW		0xffff,0x0000,0x9200,0x00cf	; 撉傒彂偒壜擻僙僌儊儞僩32bit
		DW		0xffff,0x0000,0x9a28,0x0047	; 幚峴壜擻僙僌儊儞僩32bit乮bootpack梡乯

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack:
