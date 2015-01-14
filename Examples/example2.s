.global main
.global _irq_handler
.global fin

		
.section ".vector", "ax"
	movs pc, lr
	b swi
	subs pc, lr, #4
	subs pc, lr, #4
	b main
	b _irq_handler
	subs pc, lr, #4
		
.text

main:
	mov r0, #0x12
	mov r1, #0x34
	add r0, r1, r0, lsl #8
	mov r1, #0x56
	add r0, r1, r0, lsl #8
	mov r1, #0x78
	add r0, r1, r0, lsl #8
	mov r1, #0x2000
	str r0, [r1]
	ldrb r2, [r1]
	add r1, r1, #3
	ldrb r3, [r1]
fin:
	swi 0x123456

_irq_handler:
	mov r2, #0x42
	sub r8, r2, #1
	subs pc, lr, #4

swi:
	swi 0x123456
	movs pc, lr
	
