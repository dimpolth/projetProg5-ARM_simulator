.global main		
.text

main:
	mov r0, #0xC4
	adcs r1, r0, r1
	add r2, r0, r1
	and r3, r1, #0xF 	@resultat attendu : 4
	bic r4, r1, #0xF 	@resultat attendu : C0
	cmn r4, #0xC0		@si le resultat de la comparaison n'est pas le bon, on saute directement à la fin du programme
	beq end
	cmp r4, #0xC0	
	bne end
	eor r5, r4, #0xC6 	@resultat attendu : 6
	mvn r6, #0x6 		@resultat attendu : FFFFFFF9
	orr r7, r4, #0xC6 	@resultat attendu : C6
	rsb r8, r5, #0xF3	@resultat attendu : ED
	rsc r9, r8, #0xF3	@resultat attendu : 6
	teq r5, r9
	bne end
	tst r5, #0
	bne end
	sbc r10, r8, #0xD	@resultat attendu : E0
	sub r11, r7, #0xD	@resultat attendu : B9
end:
	swi 0x123456

	
