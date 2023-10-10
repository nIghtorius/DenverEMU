; *******************************
; ** Global NES PPU code
; ** written by Peter Santing

initppu:				; initializes the graphicschip of the NES incl. WARMUP
	ldx		#$00
	stx		$2000		; write $00 to reg $2000 (addr-increment is 1)
	stx 	$2001
	bit		$2002
-	bit		$2002
	bpl		-
-	bit		$2002
	bpl		-
	rts
	
load_raw_ppu_data:		; input ZPAGE: 00, 01 == ADDR NAMETABLE
						;			   02, 03 == ADDR ATTRIB	
	
	;	Nametable Data.
	lda		#$20
	sta		$2006
	lda		#$00
	sta		$2006
	ldy		#0		
	ldx		#4
-	lda		($00), y
	sta		$2007
	iny
	bne		-
	inc		$01
	dex
	bne		-
	;	Attribute Data.
	lda		#$23
	sta		$2006
	lda		#$c0
	sta		$2006
	ldy		#$00
-	lda		($02), y
	sta		$2007
	iny
	cpy		#$41
	bne		-
	rts

	; Macro definition
	MACRO	LoadScene	NameTable,	AttributeTable
		lda		#<NameTable
		sta		$00
		lda		#>NameTable
		sta		$01
		lda		#<AttributeTable
		sta		$02
		lda		#>AttributeTable
		sta		$03
		jsr		load_raw_ppu_data
	ENDM
	
LoadCompressedScene:
	;	Nametable Data.
		clc
		lda		$02
		sta		$2006
		lda		$03
		sta		$2006
		ldy		#0	
-		lda		($00), y		; load repeat value (1--255, 0=EOP)
		beq		+++
		tax						; store this in X
		iny						; increment Y
		beq 	+				; increase page.
---		lda		($00), y		; load tile
--		sta		$2007
		dex
		bne 	--				; loop until X = 0!
		iny
		beq		++				; increase page
----	bne		-				; start over again.
+		inc		$01
		bcc		---
++		inc		$01
		bcc		----
+++		rts

	; Macro definition
	MACRO	LoadRLEScene	TableAddr, Graphic
		lda		#<Graphic
		sta		$00
		lda		#>Graphic
		sta		$01
		lda		#<TableAddr
		sta		$03
		lda		#>TableAddr
		sta		$02
		jsr		LoadCompressedScene
	ENDM
	
LoadPal:		; load palette (PTR = $00)
	lda		#$3F
	sta		$2006
	lda		#$00
	sta		$2006
	ldy		#$00
-	lda		($00), y
	sta		$2007
	iny
	cpy		#$20
	bne		-
	rts

	MACRO	LoadPalette	PaletteData
		lda		#<PaletteData
		sta		$00
		lda		#>PaletteData
		sta		$01
		jsr		LoadPal
	ENDM
	
Enable_PPU_Rendering:
	lda		$2002
	lda		#$1E
	sta		$2001
	rts
	
Disable_PPU_Rendering:
	lda		$2002
	lda		#$00
	sta		$2001
	rts
	
Reset_Scroll_Regs:
	lda		#$00
	sta		$2005
	sta		$2005
	rts
	
	MACRO	SetScroll	x, y
		lda	#x
		sta	$2005
		lda	#y
		sta	$2005
	ENDM
	
	MACRO	SetScrollEx	y, x
		lda		#$00
		sta		$2006
		lda		#x
		and		#%11000111
		sta		$2005
		lda		#y
		tax
		and		#%00000111
		sta		$2005
		lda		#x
		and		#%00111000
		asl
		asl
		sta		$00
		txa
		and		#%11111000
		lsr
		lsr
		lsr
		and		$00
		sta		$2006
	ENDM
	
Enable_NMI:
	lda		#$88
	sta		$2000
	rts
	
Reset_OAM:
	ldx		#$00
	lda		#$F0
-	sta		$0500, x
	inx
	bne 	-
	rts
	
	MACRO	SetSprite	index, xpos, ypos, tile, attr
		SprY = $0500 + index * 4
		SprX = SprY + 3
		SprT = SprY + 1
		SprA = SprY + 2
		lda		#xpos
		sta		SprX
		lda		#ypos
		sta		SprY
		lda		#attr
		sta		SprA
		lda		#tile
		sta		SprT
	ENDM
	
Update_Sprite_Buffer:
	lda		#$00
	sta		$2003
	lda		#$05
	sta		$4014 ;		OAM DMA.
	rts

Write_Text:	;; write text (pointer $0000), PPU-address need to be set. UNSAFE registers are lost.
		ldy		#0
-		lda		($00), y
		beq		+
		cmp		#32		;space
		beq		++
		cmp		#65		;A-Z		
		bcs		+++
		cmp		#$30	;0-9
		bcs		++++
--		sta		$2007
		iny
		clc
		bcc		-
+		rts
++		lda		#0
		bcs		--		; special case space #32
+++		sbc		#$31
		bcs		--		; special case "TEXT"
++++	sbc		#$30
		bcs		--		; special case "01234"		


	MACRO	WriteText_XY x, y, text
		Addr = $2000 + y * 32 + x
		lda #>Addr
		sta	$2006
		lda #<Addr
		sta	$2006
		lda	#<text
		sta	$00
		lda	#>text
		sta $01
		jsr	Write_Text
	ENDM
	
	MACRO	WriteText_Addr Addr, text
		SetPPUAddr		Addr
		lda	#<text
		sta	$00
		lda	#>text
		sta $01
		jsr	Write_Text
	ENDM	
	
	MACRO	WriteText text
		lda #<text
		sta $00
		lda #>text
		sta $01
		jsr	Write_Text
	ENDM
	
	MACRO	SetPosition x, y
		Addr = $2000 + y * 32 + x
		lda #>Addr
		sta $2006
		lda #<Addr
		sta $2006
	ENDM
	
	MACRO	SetTextPointer text
		lda #<text
		sta $00
		lda #>text
		sta $01
	ENDM		
	