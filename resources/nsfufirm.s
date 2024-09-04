; uFirmware (loaded on 0x3000) for the Denver NSF player.
; Denver emulates a "hardware" type NSF player.

; 09-05-2024, Remove PPU dependency. the NSF rom code has it's own NMI timer now. Supporting all NSF speeds now.
; patched by jumping directly to the trackselect code after setting up CPU/STACK.
; also writes 0x00 to 0x2000 instead of 0x80 ( thus disabling NMI on the PPU )

; 25-08-2024, Cleaned up source. Code that isn't used has been removed.
; update code for the NMI toggle. (nmi for play routine needs to be toggled on after song init)
; this can be done by writing 0xFF to memory address 0x3000
; Reason for NMI toggle: so that the NMI doesn't interfere with the initialization process. Getting the player in an invalid state.

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
	ldx #$40
	stx $4017
	ldx #$FF
	txs			; setup stack pointer.
	inx			; 0xFF -> 0x00
	lda	#$00
	sta 	$2000	; disable nmi PPU emulation.

trackselect:
	lda	#$00	; first track (patchable)
	ldx	#$00	; ntsc timing.

init:
	jsr	$8000	; init function (patched by loader)
	lda	#$ff	; setup Denvers NSF mode (0x3000) == enable NMI.
	sta	$3000	; setup DEnvers NSF mode (0x3000) == enable NMI.

lockloop:
	jmp	lockloop	; lock up (only NMI is running, NSF interface will reset PC to RESET when new song is selected)


nmi:
play:
	jsr	$8000		; patched (play routine)
irq:
	rti			; return from IRQ/NMI

