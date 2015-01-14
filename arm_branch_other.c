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
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"
#include <debug.h>
#include <stdlib.h>


int arm_branch(arm_core p, uint32_t ins) {
	
	// B / BL
		// R�cup�ration du PC actuel
	int PC = arm_read_register(p, 15);
		// Enregistrement du PC si le brachement est link�
	if (get_bit(ins, 24) == 1) {
		arm_write_register(p, 14, PC-4);
	}
		// Traitement de l'offset (de 24 � 32 bit)
	int offset = asr(ins << 8, 8) * 4;
		// Modification du PC
	int newPC = PC + offset;
	arm_write_register(p, 15, (uint32_t) newPC);

	return 0;
}

int arm_coprocessor_others_swi(arm_core p, uint32_t ins) {

	// SWI
	if (get_bit(ins, 24)) {
		if ((ins & 0xFFFFFF) == 0x123456)
			exit(0);
		return SOFTWARE_INTERRUPT;
	} 
	
	return UNDEFINED_INSTRUCTION;
}

int arm_miscellaneous(arm_core p, uint32_t ins) {

	// MRS
	if (get_bits(ins,27,23) == 2 && get_bits(ins,21,20) == 0) { 
		if (get_bit(ins,22) == 0) // CPSR -> Rd
			arm_write_register(p,get_bits(ins,15,12),arm_read_cpsr(p));
		else			  // SPSR -> Rd
			arm_write_register(p,get_bits(ins,15,12),arm_read_spsr(p));
		return 0;
	}

	// STRH, LDRH (+ �ventuellement autres load/store sp�ciaux)
	if(get_bit(ins, 7) && get_bit(ins, 4)){
		uint32_t address = 0;
		uint8_t P = get_bit(ins, 24);
		uint8_t U = get_bit(ins, 23);
		uint8_t I = get_bit(ins, 22);
		uint8_t W = get_bit(ins, 21);
		uint8_t L = get_bit(ins, 20);
		uint8_t S = get_bit(ins, 6);
		uint8_t H = get_bit(ins, 5);
		uint32_t rm = arm_read_register(p, get_bits(ins, 3, 0));
		uint32_t rn = arm_read_register(p, get_bits(ins, 19, 16));
		uint8_t rd = get_bits(ins, 15, 12);
		uint8_t immedH = get_bits(ins, 11, 8);
		uint8_t immedL = get_bits(ins, 3, 0);

		if(P && I && !W){
			uint8_t offset8 = (immedH << 4) | immedL;
			if(U)
				address = rn + offset8;
			else address = rn - offset8;
		}

		if(P && !I && !W){
			if(U)
				address = rn + rm;
			else address = rn - rm;
		}

		if(P && I && W){
			uint8_t offset8 = (immedH << 4) | immedL;
			if(U)
				address = rn + offset8;
			else address = rn - offset8;
			arm_write_register(p, rn, address);
		}

		if(P && !I && W){
			if(U)
				address = rn + rm;
			else address = rn - rm;
			arm_write_register(p, rn, address);
		}
	
		if(!P && I && !W){
			address = rn;
			uint8_t offset8 = (immedH << 4) | immedL;
			if(U)
				address = rn + offset8;
			else address = rn - offset8;
		}

		if(!P && !I && !W){
			address = rn;
			if(U)
				address = rn + rm;
			else address = rn - rm;
		}
		
		int erreur=0;
		if(L){ // Load
			if (!S && H){ // LDRH
				uint16_t res;
				//printf("LDRH, address : %x \n",address);
				erreur = arm_read_half(p, address, &res);
				erreur = arm_write_register(p, rd, res);
			}
		}
		else { // Store
			if (!S && H){ // STRH
				uint16_t res;
				//printf("STRH, address : %x \n",address);
				res = arm_read_register(p, rd);
				erreur = arm_write_half(p, address, res);
			}
		}
		return erreur;
	}

	return UNDEFINED_INSTRUCTION;
}
