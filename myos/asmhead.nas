; haribote-os boot asm
; TAB=4
[INSTRSET "i486p"]

VBEMODE	EQU		0x101			; 


;	0x101 :  640 x  480 x 8bitｷﾖｱ貭ﾊ
;	0x103 :  800 x  600 x 8bitｷﾖｱ貭ﾊ
;	0x105 : 1024 x  768 x 8bitｷﾖｱ貭ﾊ
;	0x107 : 1280 x 1024 x 8bitｷﾖｱ貭ﾊ

BOTPAK	EQU		0x00280000		; bootpackﾄﾚｴ豬ﾘﾖｷ
DSKCAC	EQU		0x00100000		; 
DSKCAC0	EQU		0x00008000		; ディスクキャッシュの場所（リアルモード）

; BOOT_INFOﾐﾅﾏ｢
CYLS	EQU		0x0ff0			; 
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			; 
SCRNX	EQU		0x0ff4			; 解像度のX
SCRNY	EQU		0x0ff6			; 解像度のY
VRAM	EQU		0x0ff8			; グラフィックバッファの開始番地

		ORG		0xc200			; このプログラムがどこに読み込まれるのか

; ﾉ雜ｨｻｭﾃ貽｣ﾊｽ,VESA BIOS extension ﾖﾆﾗ�ﾗｨﾒｵｵﾄBIOS｣ｬﾒﾔｴ�ｵｽｲｻﾍｬﾏﾔｿｨｵﾄｼ貶ﾝ｡｣ﾀ�ﾓﾃﾋ�ｿﾉﾒﾔﾊｹﾓﾃﾏﾔｿｨｵﾄｸﾟｷﾖｱ貭ﾊ｡｣
;ax=4f02｣ｬbx=ｻｭﾃ貽｣ﾊｽ



;ﾖﾐｶﾏ10ｺﾅｵﾄ4Fxxﾏﾂｵﾄｺｯﾊ�
｡｡;AX=4F00 -ｻ�ﾈ｡VESAｽ盪ｹﾐﾅﾏ｢
｡｡｡｡; 01 -ｻ�ﾈ｡ﾄｳﾖﾖﾏﾔﾊｾﾄ｣ﾊｽｵﾄﾊ�ﾐﾔﾐﾅﾏ｢
｡｡｡｡ ;02 -ﾉ靹ﾃﾏﾔﾊｾﾄ｣ﾊｽ
｡｡｡｡ ;03 -ｻ�ﾈ｡ｵｱﾇｰｵﾄﾄ｣ﾊｽ
｡｡｡｡ ;05 -ﾒｳﾇﾐｻｻ(ﾓﾃﾓﾚｶﾔﾏﾔﾊｾﾄﾚｴ贍ｱｽﾓｲﾙﾗ�ｻｭｵ�,ﾏﾟ)
｡｡｡｡;ﾆ萢�ｵﾄﾈ�ｿﾚ,ｳ�ｿﾚｲﾎﾊ�ｿﾉｲﾎｿｴﾖﾐｶﾏﾊﾖｲ�


; ﾈｷﾈﾏvbeﾊﾇｷ�ｴ贇ﾚ
		MOV		AX,0x9000
		MOV		ES,AX
		MOV		DI,0
		MOV		AX,0x4f00;ｻ�ﾈ｡ﾏﾔﾊｾﾄ｣ﾊｽ
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn320

; ｼ�ｲ餬BEｵﾄｰ豎ｾ

		MOV		AX,[ES:DI+4]
		CMP		AX,0x0200
		JB		scrn320			; if (AX < 0x0200) goto scrn320

; ｻ�ﾈ｡ｻｭﾃ貽｣ﾊｽﾐﾅﾏ｢

		MOV		CX,VBEMODE
		MOV		AX,0x4f01
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn320

; ｻｭﾃ貽｣ﾊｽｵﾄﾈｷﾈﾏ

		CMP		BYTE [ES:DI+0x19],8 ;ﾑﾕﾉｫﾊ�ｱﾘﾐ�ﾎｪ8
		JNE		scrn320
		CMP		BYTE [ES:DI+0x1b],4 ;ﾑﾕﾉｫｵﾄﾖｸｶｨｷｽｷｨ
		JNE		scrn320
		MOV		AX,[ES:DI+0x00] ;ﾄ｣ﾊｽﾊ�ﾐﾔ
		AND		AX,0x0080
		JZ		scrn320			; bit7ｲｻﾊﾇｾﾍｲｻｺﾃｰ�

; ｻｭﾃ貽｣ﾊｽｵﾄﾇﾐｻｻ

		MOV		BX,VBEMODE+0x4000;ｻｭﾃ貽｣ﾊｽﾖｵ+00x4000
		MOV		AX,0x4f02
		INT		0x10
		MOV		BYTE [VMODE],8	; ｼﾇﾏﾂｻｭﾃ貽｣ﾊｽ
		MOV		AX,[ES:DI+0x12]
		MOV		[SCRNX],AX
		MOV		AX,[ES:DI+0x14]
		MOV		[SCRNY],AX
		MOV		EAX,[ES:DI+0x28]
		MOV		[VRAM],EAX
		JMP		keystatus

scrn320:
		MOV		AL,0x13			; VGAﾍｼ｣ｬ320x200x8bitｲﾊﾉｫ
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8	; ｼﾇﾏﾂｻｭﾃ貽｣ﾊｽ
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000

keystatus:
		MOV		AH,0x02
		INT		0x16 			; keyboard BIOS
		MOV		[LEDS],AL

;ｽ�ﾖｹpic0｣ｬpic1,ﾋ�ﾓﾐﾖﾐｶﾏ

		MOV		AL,0xff
		OUT		0x21,AL
		NOP						
		OUT		0xa1,AL

		CLI						

; ﾎｪﾁﾋﾈﾃCPUﾄﾜｹｻｷﾃﾎﾊ1MBﾄﾚｴ豼ﾕｼ腆20GATE
;ｴﾓﾀ�ﾂﾛﾉﾏｽｲ｣ｬｴ�ｿｪA20 Gateｵﾄｷｽｷｨﾊﾇﾍｨｹ�ﾉ靹ﾃ8042ﾐｾﾆｬﾊ莎�ｶﾋｿﾚ｣ｨ64h｣ｩｵﾄ2nd-bit｣ｬ
;ｵｫﾊﾂﾊｵﾉﾏ｣ｬｵｱﾄ耘�8042ﾐｾﾆｬﾊ莎�ｶﾋｿﾚｽ�ﾐﾐﾐｴｲﾙﾗ�ｵﾄﾊｱｺ�｣ｬ
;ﾔﾚｼ�ﾅﾌｻｺｳ衂�ﾖﾐ｣ｬｻ�ﾐ�ｻｹﾓﾐｱ�ｵﾄﾊ�ｾﾝﾉﾐﾎｴｴｦﾀ�｣ｬﾒ�ｴﾋﾄ羈ﾘﾐ�ﾊﾗﾏﾈｴｦﾀ�ﾕ簟ｩﾊ�ｾﾝ｡｣
		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL
		CALL	waitkbdout

; ﾇﾐｻｻｵｽｱ｣ｻ､ﾄ｣ﾊｽ

[INSTRSET "i486p"]				; ﾊｹﾓﾃ486ﾖｸﾁ�

		LGDT	[GDTR0]			; ﾉ雜ｨﾁﾙﾊｱGDT
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; bit31ｽ�ﾖｹｷﾖﾒｳ
		OR		EAX,0x00000001	; bit0 ﾎｪ1 ﾇﾐｻｻｵｽｱ｣ｻ､ﾄ｣ﾊｽ
		MOV		CR0,EAX
		JMP		pipelineflush
pipelineflush:
		MOV		AX,1*8			;  ｿﾉｶﾁﾐｴｵﾄｶﾎ
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; bootpackｴｫﾋﾍ

		MOV		ESI,bootpack	; ｴｫﾋﾍﾔｴ
		MOV		EDI,BOTPAK		; ｴｫﾋﾍﾄｿｵﾄｵﾘ0x00280000
		MOV		ECX,512*1024/4  ;ｴ�ﾐ｡ﾎｪ512KB
		CALL	memcpy

; 

; ﾊﾗﾏﾈｴﾓﾆ�ｶｯﾉﾈﾇ�ｿｪﾊｼ

		MOV		ESI,0x7c00		; ﾔｴ
		MOV		EDI,DSKCAC		; ﾄｿｵﾄｵﾘ 1M 0x10000
		MOV		ECX,512/4
		CALL	memcpy

; ﾋ�ﾓﾐﾊ｣ﾏﾂｵﾄ

		MOV		ESI,DSKCAC0+512	; ﾔｴ
		MOV		EDI,DSKCAC+512	; ﾄｿｵﾄ 0x100200
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; ｴﾓﾖ�ﾃ賁�ｱ莉ｻｳﾉﾗﾖｽﾚﾊ�/4
		SUB		ECX,512/4		; ｼ�ﾈ･ipl
		CALL	memcpy

; asmhead ﾍ�ｳﾉｵﾄｹ､ﾗ�ﾖﾁｴﾋﾒﾑﾍ�ｳﾉ
;	ﾏﾂﾃ贊ﾐbootpackﾍ�ｳﾉ

; bootpackﾆ�ｶｯ

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; ﾃｻﾓﾐﾒｪｴｫﾋﾍｵﾄｶｫﾎ�ﾊｱ
		MOV		ESI,[EBX+20]	; ｴｫﾋﾍﾔｴ
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; ｴｫﾋﾍﾄｿｵﾄｵﾘ
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; ｳ�ﾊｼｶﾑﾕｻ
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		; ﾇ蠢ﾕﾊ�ｾﾝｽﾓﾊﾕｻｺｳ衂�ｵﾄﾀｬｻ�ﾊ�ｾﾝ
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
		DW		0xffff,0x0000,0x9200,0x00cf	; ｿﾉｶﾁﾐｴｵﾄｶﾎ 32bit
		DW		0xffff,0x0000,0x9a28,0x0047	; ｿﾉﾒﾔﾖｴﾐﾐｵﾄｶﾎ

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack:
