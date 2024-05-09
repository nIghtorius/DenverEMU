; uFirmware (loaded on 0x3000) for the Denver NSF player.
; Denver emulates a "hardware" type NSF player.

; 09-05-2024, Remove PPU dependency. the NSF rom code has it's own NMI timer now. Supporting all NSF speeds now.
; patched by jumping directly to the trackselect code after setting up CPU/STACK.
; also writes 0x00 to 0x2000 instead of 0x80 ( thus disabling NMI on the PPU )

base	$3000

; init vectors.

.dw		reset				; contains the "start" address
.dw		nmi				; contains the "nmi" function
.dw		irq				; contains the "irq" function, but it is officially not used.
.dw		trackselect + 1			; edit this byte to change track.
.dw		init + 1			; init
.dw		play + 1			; play

reset:
	; initialize uFirmware.
	sei
	ldx #40
	stx $4017
	ldx $FF
	txs			; setup stack pointer.
	
	; we are going to use the ppu emulation for the timings. (NTSC)
	inx			; 0xFF -> 0x00
	jmp trackselect		; stx $2000		; 0 -> 0x2000, 0x2001
	stx $2001		
	
	; wait for the ppu to be "ready"
	bit $2002
-	bit $2002
	bpl -
-	bit $2002
	bpl -
	; ppu ready.

	; activate rendering (rendering will be hidden by Denver)
	lda	$2002
	lda	#0
	sta	#2005
	lda	#$d0
	sta	$2005
	lda	#$1e
	sta	$2001	; ppu rendering on.

	; play routine.
trackselect:
	lda	#$00	; first track (patchable)
	ldx	#$00	; ntsc timing.
init:
	jsr	$8000	; init function (patched by loader)
	lda	#$00
	sta 	$2000	; disable nmi
lockloop:
	jmp	lockloop	; lock up (only NMI is running, NSF interface will reset PC to RESET when new song is selected)


nmi:
play:
	jsr	$8000		; patched (play routine)
irq:
	rti			; return from IRQ/NMI

