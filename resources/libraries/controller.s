; ***********************
; ** NES Controller code
; ** written by Peter Santing

CTRL1			EQU	$20
CTRL2			EQU $21
CTRL1_BUF		EQU $22
CTRL2_BUF		EQU $24
CTRLBUF			EQU $0A

; buttons
btnA			EQU $80
btnB			EQU $40
btnSelect		EQU $20
btnStart		EQU $10
btnUp			EQU $08
btnDown			EQU $04
btnLeft			EQU $02
btnRight		EQU $01

ReadController:	; A = controller ID, X is destroyed, Y is destroyed.
	; ACC holds controller to read.
	ldx		#01
	stx		$4016	;; strobe controller
	ldx		#00
	stx		$4016	;; for read out.
	;
	tax					;; set address to X
	lda		#0
	sta		$00, X		;; prep buffer.
	
	; 8 buttons.
	ldy		#8		
-	asl		$00, X		;; shift a bit (76543210 -> 6543210)
	lda		$4016	;; load from ctrlBUF	-- BUTTON A
	and		#1			;; filter garbage
	ora		$00, X
	sta		$00, X		;; store it.
	dey
	bne		-
	rts		

MACRO		GoReadController		base, controller
	lda		#<base
	sta		CTRLBUF
	lda 	#>base
	sta		CTRLBUF+1
	lda		#controller
	jsr		ReadController
ENDM

MACRO		TestKey			ctrl, keyid, branch
	lda		ctrl
	and		#keyid
	bne		branch	
ENDM

MACRO		TestKeySingle	ctrl, keyid, branch
	lda		ctrl
	eor		ctrl+02
	and		#keyid
	beq		++
+	lda		ctrl
	sta		ctrl+02
	and		#keyid
	bne		branch
++
ENDM
