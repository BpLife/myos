; haribote-os boot asm
; TAB=4
[INSTRSET "i486p"]

VBEMODE	EQU		0x101			; 


;	0x101 :  640 x  480 x 8bit�ֱ���
;	0x103 :  800 x  600 x 8bit�ֱ���
;	0x105 : 1024 x  768 x 8bit�ֱ���
;	0x107 : 1280 x 1024 x 8bit�ֱ���

BOTPAK	EQU		0x00280000		; bootpack�ڴ��ַ
DSKCAC	EQU		0x00100000		; 
DSKCAC0	EQU		0x00008000		; �f�B�X�N�L���b�V���̏ꏊ�i���A�����[�h�j

; BOOT_INFO��Ϣ
CYLS	EQU		0x0ff0			; 
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			; 
SCRNX	EQU		0x0ff4			; �𑜓x��X
SCRNY	EQU		0x0ff6			; �𑜓x��Y
VRAM	EQU		0x0ff8			; �O���t�B�b�N�o�b�t�@�̊J�n�Ԓn

		ORG		0xc200			; ���̃v���O�������ǂ��ɓǂݍ��܂��̂�

; �趨����ģʽ,VESA BIOS extension ����רҵ��BIOS���Դﵽ��ͬ�Կ��ļ��ݡ�����������ʹ���Կ��ĸ߷ֱ��ʡ�
;ax=4f02��bx=����ģʽ



;�ж�10�ŵ�4Fxx�µĺ���
��;AX=4F00 -��ȡVESA�ṹ��Ϣ
����; 01 -��ȡĳ����ʾģʽ��������Ϣ
���� ;02 -������ʾģʽ
���� ;03 -��ȡ��ǰ��ģʽ
���� ;05 -ҳ�л�(���ڶ���ʾ�ڴ�ֱ�Ӳ�������,��)
����;���������,���ڲ����ɲο��ж��ֲ�


; ȷ��vbe�Ƿ����
		MOV		AX,0x9000
		MOV		ES,AX
		MOV		DI,0
		MOV		AX,0x4f00;��ȡ��ʾģʽ
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn320

; ���VBE�İ汾

		MOV		AX,[ES:DI+4]
		CMP		AX,0x0200
		JB		scrn320			; if (AX < 0x0200) goto scrn320

; ��ȡ����ģʽ��Ϣ

		MOV		CX,VBEMODE
		MOV		AX,0x4f01
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn320

; ����ģʽ��ȷ��

		CMP		BYTE [ES:DI+0x19],8 ;��ɫ������Ϊ8
		JNE		scrn320
		CMP		BYTE [ES:DI+0x1b],4 ;��ɫ��ָ������
		JNE		scrn320
		MOV		AX,[ES:DI+0x00] ;ģʽ����
		AND		AX,0x0080
		JZ		scrn320			; bit7���ǾͲ��ð�

; ����ģʽ���л�

		MOV		BX,VBEMODE+0x4000;����ģʽֵ+00x4000
		MOV		AX,0x4f02
		INT		0x10
		MOV		BYTE [VMODE],8	; ���»���ģʽ
		MOV		AX,[ES:DI+0x12]
		MOV		[SCRNX],AX
		MOV		AX,[ES:DI+0x14]
		MOV		[SCRNY],AX
		MOV		EAX,[ES:DI+0x28]
		MOV		[VRAM],EAX
		JMP		keystatus

scrn320:
		MOV		AL,0x13			; VGAͼ��320x200x8bit��ɫ
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8	; ���»���ģʽ
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000

keystatus:
		MOV		AH,0x02
		INT		0x16 			; keyboard BIOS
		MOV		[LEDS],AL

;��ֹpic0��pic1,�����ж�

		MOV		AL,0xff
		OUT		0x21,AL
		NOP						
		OUT		0xa1,AL

		CLI						

; Ϊ����CPU�ܹ�����1MB�ڴ�ռ�A20GATE
;�������Ͻ�����A20 Gate�ķ�����ͨ������8042оƬ����˿ڣ�64h����2nd-bit��
;����ʵ�ϣ�������8042оƬ����˿ڽ���д������ʱ��
;�ڼ��̻������У������б��������δ���������������ȴ�����Щ���ݡ�
		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL
		CALL	waitkbdout

; �л�������ģʽ

[INSTRSET "i486p"]				; ʹ��486ָ��

		LGDT	[GDTR0]			; �趨��ʱGDT
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; bit31��ֹ��ҳ
		OR		EAX,0x00000001	; bit0 Ϊ1 �л�������ģʽ
		MOV		CR0,EAX
		JMP		pipelineflush
pipelineflush:
		MOV		AX,1*8			;  �ɶ�д�Ķ�
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; bootpack����

		MOV		ESI,bootpack	; ����Դ
		MOV		EDI,BOTPAK		; ����Ŀ�ĵ�0x00280000
		MOV		ECX,512*1024/4  ;��СΪ512KB
		CALL	memcpy

; 

; ���ȴ�����������ʼ

		MOV		ESI,0x7c00		; Դ
		MOV		EDI,DSKCAC		; Ŀ�ĵ� 1M 0x10000
		MOV		ECX,512/4
		CALL	memcpy

; ����ʣ�µ�

		MOV		ESI,DSKCAC0+512	; Դ
		MOV		EDI,DSKCAC+512	; Ŀ�� 0x100200
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; ���������任���ֽ���/4
		SUB		ECX,512/4		; ��ȥipl
		CALL	memcpy

; asmhead ��ɵĹ������������
;	������bootpack���

; bootpack����

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; û��Ҫ���͵Ķ���ʱ
		MOV		ESI,[EBX+20]	; ����Դ
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; ����Ŀ�ĵ�
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; ��ʼ��ջ
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		; ������ݽ��ջ���������������
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
		DW		0xffff,0x0000,0x9200,0x00cf	; �ɶ�д�Ķ� 32bit
		DW		0xffff,0x0000,0x9a28,0x0047	; ����ִ�еĶ�

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack:
