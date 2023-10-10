; Metasprite addon lib for PPU.s
; (c) 2014-2015 Peter Santing
;
;

; OAM is always 0x0500 in my engine so place meta sprites there.
; also NEVER TOUCH 0x00 (SPRITE #0)

	WRITEBUF	EQU $C		; zeropage address counters.
	XBUF		EQU $D;
	
	
add_meta_sprite:	; ($00) = holds metasprite addr.
					; $02 = x pos, $03 = y pos.
					; meta data is as follows. x, y, tile, bgcolor and attr
					; sprite is as follows: y, tile, attr, x

	ldx		WRITEBUF
	ldy		#0
-	lda		$02			; load x pos.
	clc
	adc		($00), Y	; add meta x pos.
	sta		XBUF
	iny
	lda		$03			; load y pos
	clc
	adc		($00), Y	; add meta y pos.
	sta		($0500), X	; save byte Y
	inx
	iny
	lda		($00), Y	; load tile number.
	sta		($0500), x	; save tile number.
	inx
	iny
	lda		($00), Y	; load attribute.
	sta		($0500), X	; save attribute.
	inx
	lda		XBUF
	sta		($0500), X	; save x position.
	iny
	inx
	lda		($00), Y	; load x pos?
	cmp		#128		; if x = 128
	bne		-			; then end loop
	stx		WRITEBUF
	rts	
	
prep_write_meta_sprites:
	lda 	#$4			; always ignore spr #0 (hard set for scrollbars, etc)
	sta		WRITEBUF
	rts
	
done_write_meta_sprites:
	lda		#0
	ldx		WRITEBUF	
	inx		; goto tile.
-	sta		($0500), X
	inx
	inx
	inx
	inx
	cpx		#$01
	bne		-
	rts
	