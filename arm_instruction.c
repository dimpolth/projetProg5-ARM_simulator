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
#include "arm_instruction.h"
#include "arm_exception.h"
#include "arm_data_processing.h"
#include "arm_load_store.h"
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"

static int arm_execute_instruction(arm_core p) {

	// Récupération de l'instruction
	uint32_t PC = arm_read_register(p, 15);
	uint32_t instr;
	if (arm_read_word(p, PC, &instr) == -1) return -1;

	// Incrémentation PC
	arm_write_register(p, 15, PC+4);

	// Récupération des registre d'état
	int n, z, c, v;
	uint32_t cpsr = arm_read_cpsr(p);
	n = get_bit(cpsr,N);
	z = get_bit(cpsr,Z);
	c = get_bit(cpsr,C);
	v = get_bit(cpsr,V);

	// vérification de la condition
	uint8_t cond = get_bits(instr,31,28);
	int res = 1;
	switch (cond) {
		case 0 : res = (z == 1); break;
		case 1 : res = (z == 0); break;
		case 2 : res = (c == 1); break;
		case 3 : res = (c == 0); break;
		case 4 : res = (n == 1); break;
		case 5 : res = (n == 0); break;
		case 6 : res = (v == 1); break;
		case 7 : res = (v == 0); break;
		case 8 : res = (c == 1 && z == 0); break;
		case 9 : res = (c == 0 && z == 1); break;
		case 10 : res = (n == v); break;
		case 11 : res = (n != v); break;
		case 12 : res = (z == 0 && n == v); break;
		case 13 : res = (z == 1 || n != v); break;
		case 14 : break;
		case 15 : break;
		default : return -1;
	}
    
  	// Execution (ou pas) de l'instruction
	if (res) {
		int type_instr = get_bits(instr,27,25);
		int deroul = 0;
		switch (type_instr) {
			case 0 :
				if(get_bits(instr, 25, 24) == 2 && get_bit(instr, 20) == 0) {
					if(get_bit(instr, 4) == 0 || (get_bit(instr, 7) == 0 && get_bit(instr, 4) == 1))
						deroul = arm_miscellaneous(p, instr);
				}
				else {
					if(get_bit(instr, 4) == 0 || (get_bit(instr, 7) == 0 && get_bit(instr, 4) == 1))
						deroul = arm_data_processing_shift(p, instr);
					else
						return -1; //Cas normalement impossible à atteindre
				}
				break;		
			case 1 : deroul = arm_data_processing_immediate_msr(p, instr); break;
			case 2 : deroul = arm_load_store(p, instr); break;
			case 3 : deroul = arm_load_store(p, instr); break;
			case 4 : deroul = arm_load_store_multiple(p, instr); break;
			case 5 : deroul = arm_branch(p, instr); break;
			case 6 : deroul = arm_coprocessor_load_store(p, instr); break;
			case 7 : deroul = arm_coprocessor_others_swi(p, instr); break;
			default : return -1; //Cas normalement impossible à atteindre
		}
		
	if (deroul == -1) return -1;
	}

	return 0;
}

int arm_step(arm_core p) {
    int result;

    result = arm_execute_instruction(p);
    if (result)
        arm_exception(p, result);
    return result;
}
