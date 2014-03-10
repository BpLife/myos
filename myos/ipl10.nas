; haribote-ipl
; TAB=4

CYLS	EQU		10				; 偳偙傑偱撉傒崬傓偐

		ORG		0x7c00			; 指明程序的装置地址 IBM规定 7c00--7dff


;一下这段是标准fat12格式软盘专用
		JMP		entry
		DB		0x90
		DB		"HARIBOTE"		; 启动区的名字
		DW		512				; 没个扇区的大小
		DB		1				; 簇的大小
		DW		1				; FAT的起始位置
		DB		2				; FAT的个数
		DW		224				; 根目录的大小
		DW		2880			; 读磁盘的大小
		DB		0xf0			; 磁盘的种类
		DW		9				; fat的长度
		DW		18				; 1个磁道有几个扇区
		DW		2				;磁头数
		DD		0				; 不使用分区
		DD		2880			; 
		DB		0,0,0x29		
		DD		0xffffffff		
		DB		"HARIBOTEOS "	
		DB		"FAT12   "		
		RESB	18				

; 程序核心

entry:
		MOV		AX,0			; 初始化寄存器
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX



		MOV		AX,0x0820
		MOV		ES,AX
		MOV		CH,0			; 柱面0
		MOV		DH,0			; 磁头0
		MOV		CL,2			; 扇区号
readloop:
		MOV		SI,0			; 幐攕夞悢傪悢偊傞儗僕僗僞
retry:
		MOV		AH,0x02			; AH=0x02 : 读盘
		MOV		AL,1			; 1扇区
		MOV		BX,0
		MOV		DL,0x00			; A驱动号
		INT		0x13			; bios int
		JNC		next			; 僄儔乕偑偍偒側偗傟偽next傊
		ADD		SI,1			; SI偵1傪懌偡
		CMP		SI,5			; SI偲5傪斾妑
		JAE		error			; SI >= 5 偩偭偨傜error傊
		MOV		AH,0x00
		MOV		DL,0x00			; A驱动号
		INT		0x13			; 充重置扇区
		JMP		retry
next:
		MOV		AX,ES			; 把内存地址后移 0x200
		ADD		AX,0x0020
		MOV		ES,AX			; 
		ADD		CL,1			; 扇区号加一
		CMP		CL,18			; 共有18个扇区
		JBE		readloop		; CL <= 18 
		MOV		CL,1
		ADD		DH,1
		CMP		DH,2
		JB		readloop		; DH < 2 磁头 
		MOV		DH,0
		ADD		CH,1
		CMP		CH,CYLS			;柱面
		JB		readloop		; CH < CYLS 

; 

		MOV		[0x0ff0],CH		; 启动区
		JMP		0xc200		;0x8200+0x4200

error:
		MOV		SI,msg
putloop:
		MOV		AL,[SI]
		ADD		SI,1			; SI偵1傪懌偡
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			; 堦暥帤昞帵僼傽儞僋僔儑儞
		MOV		BX,15			; 僇儔乕僐乕僪
		INT		0x10			; 價僨僆BIOS屇傃弌偟
		JMP		putloop
fin:
		HLT						; 壗偐偁傞傑偱CPU傪掆巭偝偣傞
		JMP		fin				; 柍尷儖乕僾
msg:
		DB		0x0a, 0x0a		; 夵峴傪2偮
		DB		"load error"
		DB		0x0a			; 夵峴
		DB		0

		RESB	0x7dfe-$		; 0x7dfe傑偱傪0x00偱杽傔傞柦椷

		DB		0x55, 0xaa
