SECTION "Main", ROM0	
	ld a, 18
	ld h, 7
	ld l, 8
	ld [HL+], a
	ld l, 8
	ld a, 0
	ld a, [HL]
