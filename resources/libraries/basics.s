TIMER			EQU	$12
TIMERINTERNAL	EQU $13

; Macros
	MACRO	PUSHA
		pha
		txa
		pha
		tya
		pha
	ENDM
	
	MACRO	POPA
		pla
		tay
		pla
		tax
		pla
	ENDM

	MACRO	Poke address, value
		lda		#value
		sta		address
	ENDM
	
	MACRO	PokePPU	address, value
		lda		#>address
		sta		$2006
		lda		#<address
		sta		$2006
		lda		#value
		sta		$2007
	ENDM
	
	MACRO	PokePPU_Unsafe address, value
		lda		#>address
		sta		$2006
		lda		#<address
		sta		$2006
		lda		#value
		sta		$2007		
	ENDM
	
	MACRO	PokePPU_Next_Unsafe value
		lda		#value
		sta		$2007
	ENDM
	
	MACRO	SetPPUAddr address
		lda		#>address
		sta		$2006
		lda		#<address
		sta		$2006
	ENDM
	
FillChar_PPU:
-	sta		$2007
	dey
	bne		-
	rts
	
	MACRO	FillCharPPU Addr, value, amount
		lda	#>address
		sta	$2006
		lda	#<address
		sta $2006
		ldy	#amount
		lda	#value
		jsr	FillChar_PPU
	ENDM

	MACRO	FillCharPPU_NA value, amount
		ldy #amount
		lda #value
		jsr	FillChar_PPU
	ENDM

ClockFourthSecondTimer:	;; destroys ACC
	dec	TIMERINTERNAL
	lda	TIMERINTERNAL
	beq	+
	rts
+	lda #15
	sta	TIMERINTERNAL
	inc	TIMER
	rts
	
ClockEighthSecondTimer; 
	dec	TIMERINTERNAL
	lda	TIMERINTERNAL
	beq +
	rts
+	lda	#7
	sta	TIMERINTERNAL
	inc	TIMER
	rts
	
SetupTimer:
	lda	#15
	sta TIMERINTERNAL
	lda	#00
	sta	TIMER
	rts
	
	