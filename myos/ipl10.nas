; haribote-ipl
; TAB=4

CYLS	EQU		10				; どこまで読み込むか

		ORG		0x7c00			; ﾖｸﾃ�ｳﾌﾐ�ｵﾄﾗｰﾖﾃｵﾘﾖｷ IBMｹ豸ｨ 7c00--7dff


;ﾒｻﾏﾂﾕ筝ﾎﾊﾇｱ�ﾗｼfat12ｸ�ﾊｽﾈ�ﾅﾌﾗｨﾓﾃ
		JMP		entry
		DB		0x90
		DB		"HARIBOTE"		; ﾆ�ｶｯﾇ�ｵﾄﾃ�ﾗﾖ
		DW		512				; ﾃｻｸ�ﾉﾈﾇ�ｵﾄｴ�ﾐ｡
		DB		1				; ｴﾘｵﾄｴ�ﾐ｡
		DW		1				; FATｵﾄﾆ�ﾊｼﾎｻﾖﾃ
		DB		2				; FATｵﾄｸ�ﾊ�
		DW		224				; ｸ�ﾄｿﾂｼｵﾄｴ�ﾐ｡
		DW		2880			; ｶﾁｴﾅﾅﾌｵﾄｴ�ﾐ｡
		DB		0xf0			; ｴﾅﾅﾌｵﾄﾖﾖﾀ�
		DW		9				; fatｵﾄｳ､ｶﾈ
		DW		18				; 1ｸ�ｴﾅｵﾀﾓﾐｼｸｸ�ﾉﾈﾇ�
		DW		2				;ｴﾅﾍｷﾊ�
		DD		0				; ｲｻﾊｹﾓﾃｷﾖﾇ�
		DD		2880			; 
		DB		0,0,0x29		
		DD		0xffffffff		
		DB		"HARIBOTEOS "	
		DB		"FAT12   "		
		RESB	18				

; ｳﾌﾐ�ｺﾋﾐﾄ

entry:
		MOV		AX,0			; ｳ�ﾊｼｻｯｼﾄｴ貳�
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX



		MOV		AX,0x0820
		MOV		ES,AX
		MOV		CH,0			; ﾖ�ﾃ�0
		MOV		DH,0			; ｴﾅﾍｷ0
		MOV		CL,2			; ﾉﾈﾇ�ｺﾅ
readloop:
		MOV		SI,0			; 失敗回数を数えるレジスタ
retry:
		MOV		AH,0x02			; AH=0x02 : ｶﾁﾅﾌ
		MOV		AL,1			; 1ﾉﾈﾇ�
		MOV		BX,0
		MOV		DL,0x00			; Aﾇ�ｶｯｺﾅ
		INT		0x13			; bios int
		JNC		next			; エラーがおきなければnextへ
		ADD		SI,1			; SIに1を足す
		CMP		SI,5			; SIと5を比較
		JAE		error			; SI >= 5 だったらerrorへ
		MOV		AH,0x00
		MOV		DL,0x00			; Aﾇ�ｶｯｺﾅ
		INT		0x13			; ｳ葷ﾘﾖﾃﾉﾈﾇ�
		JMP		retry
next:
		MOV		AX,ES			; ｰﾑﾄﾚｴ豬ﾘﾖｷｺ�ﾒﾆ 0x200
		ADD		AX,0x0020
		MOV		ES,AX			; 
		ADD		CL,1			; ﾉﾈﾇ�ｺﾅｼﾓﾒｻ
		CMP		CL,18			; ｹｲﾓﾐ18ｸ�ﾉﾈﾇ�
		JBE		readloop		; CL <= 18 
		MOV		CL,1
		ADD		DH,1
		CMP		DH,2
		JB		readloop		; DH < 2 ｴﾅﾍｷ 
		MOV		DH,0
		ADD		CH,1
		CMP		CH,CYLS			;ﾖ�ﾃ�
		JB		readloop		; CH < CYLS 

; 

		MOV		[0x0ff0],CH		; ﾆ�ｶｯﾇ�
		JMP		0xc200		;0x8200+0x4200

error:
		MOV		SI,msg
putloop:
		MOV		AL,[SI]
		ADD		SI,1			; SIに1を足す
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			; 一文字表示ファンクション
		MOV		BX,15			; カラーコード
		INT		0x10			; ビデオBIOS呼び出し
		JMP		putloop
fin:
		HLT						; 何かあるまでCPUを停止させる
		JMP		fin				; 無限ループ
msg:
		DB		0x0a, 0x0a		; 改行を2つ
		DB		"load error"
		DB		0x0a			; 改行
		DB		0

		RESB	0x7dfe-$		; 0x7dfeまでを0x00で埋める命令

		DB		0x55, 0xaa
