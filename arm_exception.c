/*
Armator - simulateur de jeu d'instruction ARMv5T � but p�dagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique G�n�rale GNU publi�e par la Free Software
Foundation (version 2 ou bien toute autre version ult�rieure choisie par vous).

Ce programme est distribu� car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but sp�cifique. Reportez-vous � la
Licence Publique G�n�rale GNU pour plus de d�tails.

Vous devez avoir re�u une copie de la Licence Publique G�n�rale GNU en m�me
temps que ce programme ; si ce n'est pas le cas, �crivez � la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
�tats-Unis.

Contact: Guillaume.Huard@imag.fr
         ENSIMAG - Laboratoire LIG
         51 avenue Jean Kuntzmann
         38330 Montbonnot Saint-Martin
*/
#include "arm_exception.h"
#include "arm_constants.h"
#include "arm_core.h"
#include "util.h"

#ifdef BIG_ENDIAN_SIMULATOR
#define CP15_reg1_EEbit 1
#else
#define CP15_reg1_EEbit 0
#endif

#define Exception_bit_9 (CP15_reg1_EEbit << 9)

void arm_exception(arm_core p, unsigned char exception) {

	uint32_t cpsr = arm_read_cpsr(p);
	int CPSR_6 = get_bit(cpsr,6) << 6;
	int CPSR_8 = get_bit(cpsr,8) << 8;
	printf("--- ------------ ---\n");
	printf("--- INTERRUPTION ---\n");

	switch (exception) {
		// VOIR PAGE A2-18 ET PLUS POUR VERIFIER LES FONCTIONS
		case RESET :
			printf("--- RESET ---\n");
			arm_write_cpsr(p, 0x1d3 | Exception_bit_9);
			arm_write_usr_register(p, 15, 0);
			break;
		case UNDEFINED_INSTRUCTION :
			printf("--- UNDEFINED_INSTRUCTION ---\n");
			arm_write_cpsr(p, (cpsr & ~0x1F) | 0x1b);
			arm_write_register(p, 14, arm_read_usr_register(p, 15));
			arm_write_spsr(p, cpsr);
			arm_write_cpsr(p, 0x09b | Exception_bit_9 | CPSR_6 | CPSR_8); 
			arm_write_usr_register(p, 15, 4);
			break;
		case SOFTWARE_INTERRUPT :
			printf("--- SOFTWARE_INTERRUPT ---\n");
			arm_write_cpsr(p, (cpsr & ~0x1F) | 0x13);
			arm_write_register(p, 14, arm_read_usr_register(p, 15));
			arm_write_spsr(p, cpsr);
			arm_write_cpsr(p, 0x093 | Exception_bit_9 | CPSR_6 | CPSR_8); 
			arm_write_usr_register(p, 15, 8);
			break;
		case PREFETCH_ABORT :
			printf("--- PREFETCH_ABORT ---\n");
			arm_write_cpsr(p, (cpsr & ~0x1F) | 0x17);
			arm_write_register(p, 14, arm_read_usr_register(p, 15)); 
			arm_write_spsr(p, cpsr);
			arm_write_cpsr(p, 0x197 | Exception_bit_9 | CPSR_6); 
			arm_write_usr_register(p, 15, 12);
			break;
		case DATA_ABORT :
			printf("--- DATA_ABORT ---\n");
			arm_write_cpsr(p, (cpsr & ~0x1F) | 0x17);
			arm_write_register(p, 14, arm_read_usr_register(p, 15)+4); 
			arm_write_spsr(p, cpsr);
			arm_write_cpsr(p, 0x197 | Exception_bit_9 | CPSR_6); 
			arm_write_usr_register(p, 15, 16);
			break;
		case INTERRUPT :
			printf("--- INTERRUPT ---\n");
			arm_write_cpsr(p, (cpsr & ~0x1F) | 0x12); 
			arm_write_register(p, 14, arm_read_usr_register(p, 15)+4);
			arm_write_spsr(p, cpsr);
			arm_write_cpsr(p, 0x192 | Exception_bit_9 | CPSR_6); 
			arm_write_usr_register(p, 15, 24);
			break;
		case FAST_INTERRUPT :
			printf("--- FAST_INTERRUPT ---\n");
			arm_write_cpsr(p, (cpsr & ~0x1F) | 0x11);
			arm_write_register(p, 14, arm_read_usr_register(p, 15)+4);
			arm_write_spsr(p, cpsr);
			arm_write_cpsr(p, 0x1d1 | Exception_bit_9); 
			arm_write_usr_register(p, 15, 28);
			break;
		default : break;
	}
	
	
}
