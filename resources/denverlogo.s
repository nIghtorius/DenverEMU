; main program to run when denver starts.
; what it does:
; 	* shows the denver logo.

;

; Header (NROM)
	.db "NES", $1A
	.db 1
	.db 1
	.db $01
	.db $00
	.fillvalue 0
	.align 16	
	
; rom start
	.fillvalue 	$FF
	.org		$C000


; Resources

include "libraries\ppu.s"

mainscreen:
	incbin "denverlogo.rle"
pal:
	incbin "denverlogo.pal"
	incbin "denverlogo.pal"

reset:
	; init cpu/stack
	sei
	ldx	#$ff
	txs

	; init ppu
	jsr initppu
	
	; load screen
	LoadRLEScene	$2000, mainscreen
	LoadPalette	pal

	jsr	Enable_PPU_Rendering
	jsr	Reset_OAM
	jsr		Enable_NMI

loop:
	jmp loop

nmi:
	SetScroll	0, 0
	jsr	Reset_Scroll_Regs
	rti


; vectors

	.fillvalue $ff
	.org $fffa
	.dw	nmi
	.dw	reset
	.dw	0

;char rom

	.fillvalue	$00
	incbin "denverlogo.chr"
	.align 8192
