SECTION "Main", ROM0	
ld A, 1
ld B, A
ld C, [HL]
ld [HL], 8
ld [HL], C
ld A, [BC]
