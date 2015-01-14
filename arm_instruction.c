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
#include "arm_instruction.h"
#include "arm_exception.h"
#include "arm_data_processing.h"
#include "arm_load_store.h"
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"

static int arm_execute_instruction(arm_core p) {

	// R�cup�ration de l'instruction et incr�mentation PC
	uint32_t instr;
	if (arm_fetch(p, &instr) == -1) return -1;


	// R�cup�ration des registre d'�tat
	int n, z, c, v;
	uint32_t cpsr = arm_read_cpsr(p);
	n = get_bit(cpsr,N);
	z = get_bit(cpsr,Z);
	c = get_bit(cpsr,C);
	v = get_bit(cpsr,V);

	// v�rification de la condition
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
	int deroul = 0;
	int8_t type_instr = get_bits(instr,27,25);
	if (res) {
		switch (type_instr) {
			case 0 :
				if(get_bits(instr, 25, 24) == 2 || (get_bit(instr, 7) == 1 && get_bit(instr, 4) == 1)) {
					if(get_bit(instr, 4) == 0 || (get_bit(instr, 7) == 0 && get_bit(instr, 4) == 1) 
								  || (get_bit(instr, 7) == 1 && get_bit(instr, 4) == 1))
					{			
						deroul = arm_miscellaneous(p, instr);
					}
				}
				else {
					if(get_bit(instr, 4) == 0 || (get_bit(instr, 7) == 0 && get_bit(instr, 4) == 1))
						deroul = arm_data_processing_shift(p, instr);
					else
						return -1; //Cas normalement impossible � atteindre
				}
				break;		
			case 1 : deroul = arm_data_processing_immediate(p, instr); break;
			case 2 : deroul = arm_load_store(p, instr); break;
			case 3 : deroul = arm_load_store(p, instr); break;
			case 4 : deroul = arm_load_store_multiple(p, instr); break;
			case 5 : deroul = arm_branch(p, instr); break;
			case 6 : deroul = arm_coprocessor_load_store(p, instr); break;
			case 7 : deroul = arm_coprocessor_others_swi(p, instr); break;
			default : return -1; //Cas normalement impossible � atteindre
		}
	}

	uint32_t PC = arm_read_register(p,15);
	
	/*printf("-- -- -- -- -- -- -- -- --\n");
	printf("-- Instruction : %x -- PC : %x --\n",instr,PC-8);
	printf("-- N : %d -- Z : %d -- C : %d -- V : %d --\n",n,z,c,v); 
	printf("-- Mode d'utilisation : %s --\n",arm_get_mode_name(cpsr & 0x1F));
	printf("-- Condition : %d - Type Instruction : %d --\n",cond,type_instr);*/
		

	return deroul;
}

int arm_step(arm_core p) {
    int result;

    result = arm_execute_instruction(p);
    if (result)
        arm_exception(p, result);
    return result;
}
