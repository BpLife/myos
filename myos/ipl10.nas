; haribote-ipl
; TAB=4

CYLS	EQU		10				; �ǂ��܂œǂݍ��ނ�

		ORG		0x7c00			; ָ�������װ�õ�ַ IBM�涨 7c00--7dff


;һ������Ǳ�׼fat12��ʽ����ר��
		JMP		entry
		DB		0x90
		DB		"HARIBOTE"		; ������������
		DW		512				; û�������Ĵ�С
		DB		1				; �صĴ�С
		DW		1				; FAT����ʼλ��
		DB		2				; FAT�ĸ���
		DW		224				; ��Ŀ¼�Ĵ�С
		DW		2880			; �����̵Ĵ�С
		DB		0xf0			; ���̵�����
		DW		9				; fat�ĳ���
		DW		18				; 1���ŵ��м�������
		DW		2				;��ͷ��
		DD		0				; ��ʹ�÷���
		DD		2880			; 
		DB		0,0,0x29		
		DD		0xffffffff		
		DB		"HARIBOTEOS "	
		DB		"FAT12   "		
		RESB	18				

; �������

entry:
		MOV		AX,0			; ��ʼ���Ĵ���
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX



		MOV		AX,0x0820
		MOV		ES,AX
		MOV		CH,0			; ����0
		MOV		DH,0			; ��ͷ0
		MOV		CL,2			; ������
readloop:
		MOV		SI,0			; ���s�񐔂𐔂��郌�W�X�^
retry:
		MOV		AH,0x02			; AH=0x02 : ����
		MOV		AL,1			; 1����
		MOV		BX,0
		MOV		DL,0x00			; A������
		INT		0x13			; bios int
		JNC		next			; �G���[�������Ȃ����next��
		ADD		SI,1			; SI��1�𑫂�
		CMP		SI,5			; SI��5���r
		JAE		error			; SI >= 5 ��������error��
		MOV		AH,0x00
		MOV		DL,0x00			; A������
		INT		0x13			; ����������
		JMP		retry
next:
		MOV		AX,ES			; ���ڴ��ַ���� 0x200
		ADD		AX,0x0020
		MOV		ES,AX			; 
		ADD		CL,1			; �����ż�һ
		CMP		CL,18			; ����18������
		JBE		readloop		; CL <= 18 
		MOV		CL,1
		ADD		DH,1
		CMP		DH,2
		JB		readloop		; DH < 2 ��ͷ 
		MOV		DH,0
		ADD		CH,1
		CMP		CH,CYLS			;����
		JB		readloop		; CH < CYLS 

; 

		MOV		[0x0ff0],CH		; ������
		JMP		0xc200		;0x8200+0x4200

error:
		MOV		SI,msg
putloop:
		MOV		AL,[SI]
		ADD		SI,1			; SI��1�𑫂�
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			; �ꕶ���\���t�@���N�V����
		MOV		BX,15			; �J���[�R�[�h
		INT		0x10			; �r�f�IBIOS�Ăяo��
		JMP		putloop
fin:
		HLT						; ��������܂�CPU���~������
		JMP		fin				; �������[�v
msg:
		DB		0x0a, 0x0a		; ���s��2��
		DB		"load error"
		DB		0x0a			; ���s
		DB		0

		RESB	0x7dfe-$		; 0x7dfe�܂ł�0x00�Ŗ��߂閽��

		DB		0x55, 0xaa
