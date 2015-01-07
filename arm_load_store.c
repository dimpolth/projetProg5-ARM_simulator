/*
Armator - simulateur de jeu d'instruction ARMv5T à but pédagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique Générale GNU publiée par la Free Software
Foundation (version 2 ou bien toute autre version ultérieure choisie par vous).

Ce programme est distribué car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but spécifique. Reportez-vous à la
Licence Publique Générale GNU pour plus de détails.

Vous devez avoir reçu une copie de la Licence Publique Générale GNU en même
temps que ce programme ; si ce n'est pas le cas, écrivez à la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
États-Unis.

Contact: Guillaume.Huard@imag.fr
         ENSIMAG - Laboratoire LIG
         51 avenue Jean Kuntzmann
         38330 Montbonnot Saint-Martin
*/
#include "arm_load_store.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "util.h"
#include "memory.h"
#include "debug.h"

int arm_load_store(arm_core p, uint32_t ins) {
	uint32_t address = 0;
	uint32_t index = 0;
	uint32_t rm = arm_read_register(p, get_bits(ins, 3, 0));
	uint32_t rn = arm_read_register(p, get_bits(ins, 19, 16));
	uint32_t rd = arm_read_register(p, get_bits(ins, 15, 12));
	uint8_t cond = get_bits(ins, 31, 28);
	uint8_t shift = get_bits(ins, 6, 5);
	uint8_t shift_imm = get_bits(ins, 11, 7);
	uint8_t I = get_bit(ins, 25);
	uint8_t P = get_bit(ins, 24);
	uint8_t U = get_bit(ins, 23);
	uint8_t B = get_bit(ins, 22);
	uint8_t W = get_bit(ins, 21);
	uint8_t L = get_bit(ins, 20);
	uint16_t offset = get_bits(ins, 11, 0);
	
	if (!I && P && !W){ // immediate offset
		if (U)
			address = rn + offset;
		else
			address = rn - offset;
	}

	if (I && P && !W){ 
		if(!get_bits(ins, 11, 4)){ // register offset
			if (U)
				address = rn + rm;
			else
				address = rn - rm;	
		}
		else {
		index = scaled_switch(p, rm, shift, shift_imm); // scaled register offset
		if (U)
			address = rn + index;
		else
			address = rn - index;
		}
	}

	if(!I && P && W){ // immediate pre_indexed
		if (U)
			address = rn + offset;
		else
			address = rn - offset;
		if (cond) arm_write_register(p, rn, address);
	}
	
	if(I && P && W){
		if(!get_bits(ins, 11, 4)){ // register pre-indexed
			if (U)
				address = rn + index;
			else
				address = rn - index;
		}
		else { // scaled register pre-indexed
		index = scaled_switch(p, rm, shift, shift_imm); 
		if (U)
			address = rn + index;
		else
			address = rn - index;
		}
		if (cond) arm_write_register(p, rn, address);
	}

	if(!I && !P && !W){ // immediate post-indexed
		address = rn;
		if (cond) arm_write_register(p, rn, rn+offset);
		else arm_write_register(p, rn, rn-offset);
	}

	if(I && !P && !W){
		if(!get_bits(ins, 11, 4)){ // register post-indexed
			address = rn;
			if (cond) arm_write_register(p, rn, rn+rm);
			else arm_write_register(p, rn, rn-rm);
		}
		else { // scaled register post-indexed
			address = rn;
			index = scaled_switch(p, rm, shift, shift_imm);
			if (cond) arm_write_register(p, rn, rn+index);
			else arm_write_register(p, rn, rn-index);
		}
	}

	if(!address) return UNDEFINED_INSTRUCTION;
	int erreur;
	if(L){ // Load
		if (B){ // LDRB
			uint8_t res;
			erreur = arm_read_byte(p, address, &res);
			erreur = arm_write_register(p, rd, res);
		}
		else { // LDR
			uint32_t res;
			erreur = arm_read_word(p, address, &res);
			erreur = arm_write_register(p, rd, res);
		}
	}
	else { // Store
		if (B){ // STRB
			uint8_t res;
			res = arm_read_register(p, rd);
			erreur = arm_write_byte(p, address, res);
		}
		else { // STR
			uint32_t res;
			res = arm_read_register(p, rd);
			erreur = arm_write_word(p, address, res);
		}
	}
	return erreur;
}

uint32_t scaled_switch(arm_core p, uint32_t rm, uint8_t shift, uint8_t shift_imm){ 
	// Switch utilisé à plusieurs reprises que nous avons donc mis dans une sous-fonction
	uint32_t index = 0;
	switch (shift){
		case 0: index = rm << shift_imm;
				break;
		case 1: if(shift_imm == 0)
					index = 0;
				else index = rm >> shift_imm;
				break;
		case 2: if(shift_imm == 0){
					if(get_bit(rm, 31))
						index = 0xFFFFFFFF;
					else index = 0;
				}
				else index = asr(rm, shift_imm);
				break;
		case 3: if(shift_imm == 0){
					uint32_t cpsr = arm_read_cpsr(p);
					uint8_t c = get_bit(cpsr, C);
					index = (c << 31) | (rm >> 1);
				} else index = ror(rm, shift_imm);
				break;
	}
	return index;
} 

int arm_load_store_multiple(arm_core p, uint32_t ins) {
	uint32_t start_address = 0;
	uint32_t end_address = 0;
	uint32_t register_list = arm_read_register(p, get_bits(ins, 15, 0));
	uint32_t rn = arm_read_register(p, get_bits(ins, 19, 16));
	uint8_t cond = get_bits(ins, 31, 28);
	uint8_t P = get_bit(ins, 24);
	uint8_t U = get_bit(ins, 23);
	uint8_t W = get_bit(ins, 21);
	uint8_t L = get_bit(ins, 20);

	if(!P && U){ // increment after
		start_address = rn;
		end_address = rn + (number_of_set_bits_in(register_list)*4)-4;
		if(cond && W==1)
			rn = rn + (number_of_set_bits_in(register_list)*4);
	}

	if(P && U){ // increment before
		start_address = rn + 4;
		end_address = rn + (number_of_set_bits_in(register_list)*4);
		if(cond && W==1)
			rn = rn + (number_of_set_bits_in(register_list)*4);
	}

	if(!P && !U){ // decrement after
		end_address = rn;
		start_address = rn - (number_of_set_bits_in(register_list)*4)+4;
		if(cond && W==1)
			rn = rn - (number_of_set_bits_in(register_list)*4);
	}

	if(P && !U){ // decrement before
		end_address = rn - 4;
		start_address = rn - (number_of_set_bits_in(register_list)*4);
		if(cond && W==1)
			rn = rn - (number_of_set_bits_in(register_list)*4);
	}
	
	if(!start_address || !end_address) return UNDEFINED_INSTRUCTION;

	arm_write_register(p, get_bits(ins, 19, 16), rn);

	uint32_t res;
	uint32_t address;
	int erreur = 0;
	int i = 0;
	if(L){ // LDM(1)
		for(address = start_address; address <= end_address; address+=4){
			erreur = arm_read_word(p, address, &res);
			erreur = arm_write_register(p, get_next_register(register_list, i), res);
			i++;
		}
	}
	else { // STM(1)
		for(address = start_address; address <= end_address; address+=4){
			res = arm_read_register(p, get_next_register(register_list, i));
			erreur = arm_write_word(p, address, res);
			i++;
		}
	}
	
    return erreur;
}

int number_of_set_bits_in(uint16_t register_list){
	int i;
	int res = 0;
	for(i=0; i<16; i++){
		if(get_bit(register_list, i))
			res++;
	}
	return res;
}

int get_next_register(uint16_t register_list, int num){
	int i;
	int cpt = 0;
	for(i=0; i<16; i++){
		if(get_bit(register_list, i)){
			if(cpt==num)
				return i;
			cpt++;
		}
	}
	return -1;
}

int arm_coprocessor_load_store(arm_core p, uint32_t ins) {
    /* Not implemented */
    return UNDEFINED_INSTRUCTION;
}
