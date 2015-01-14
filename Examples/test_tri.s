.global main
.text
init:
	mov r10, #0x85
    mov r11, #0x34
    add r10, r11, r10, lsl #8
    mov r11, #0x12
    add r10, r11, r10, lsl #8
    mov r11, #0x78
    add r10, r11, r10, lsl #8
	
	mov r11, #0x9A
    mov r12, #0xFF
    add r11, r12, r11, lsl #8
    mov r12, #0x11
    add r11, r12, r11, lsl #8
    mov r12, #0x15
    add r11, r12, r11, lsl #8
    
    str r10, [r0]
    str r11, [r0,#+0x4] 	@ point d'arret ligne 21
    
    b loop
    
main:
	
    mov r0, #0xF0			@ adresse de debut du tableau 0x6F0
   	mov r3, #0xF7			@ adresse de fin du tableau 0x6F8 (car on met 8 octets)
   	mov r1, r0 				@ min <- i
    b init
loop:
	add r2, r0, #1 			@ j <- i+1
loop2:    
    ldrb r10, [r2] 			@ t[j]
    ldrb r11, [r1] 			@ t[min]
    cmp r10, r11 
    movlt r1, r2 			@ si t[j]<t[min] : min<-j
    add r2, r2, #1			@ incrementation de j
    cmp r2, r3				
    ble loop2				@ j <= n
    
    cmp r1, r0				
    bne echanger
    add r0, r0, #1 			@ incrementation de i
    cmp r0, r3
    blt loop				@ i <= n
    b end
    
echanger:
	ldrb r10, [r0] 			@ t[i]
    ldrb r11, [r1] 			@ t[min]
    strb r11, [r0]
	strb r10, [r1]
	add r0, r0, #1 			@ incrementation de i
	cmp r0, r3
    blt loop				@ i <= n
end:
	mov r8, #0xF0
	ldm r8, {r10,r11}
    swi 0x123456
.data
