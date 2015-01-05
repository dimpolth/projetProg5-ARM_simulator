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

    // R�cup�ration de l'instruction
    uint32_t PC = arm_read_register(p, 15);
    uint32_t instr;
    if (arm_read_word(p, PC, &instr) == -1) return -1;

    // R�cup�ration des registre d'�tat
    int n, z, c, v;
    uint32_t cpsr = arm_read_cpsr(p) >> 28;
    n = (cpsr & 8) >> 3;
    z = (cpsr & 4) >> 2;
    c = (cpsr & 2) >> 1;
    v = (cpsr & 1);

    // v�rification de la condition
    uint8_t cond = instr >> 28;
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
	default : return -1;
    }
    
    // Execution (ou pas) de l'instruction
    if (res) {
	int type_instr = (instr && (7 << 25)) >> 25;
	int deroul = 0;
        switch (type_instr) {
	   case 5 : deroul = arm_branch(p, instr);
	   default : return -1;
	} 
	if (deroul == -1) return -1;
    }

    // Incr�mentation PC
    arm_write_register(p, 15, PC+1);

    return 0;
}

int arm_step(arm_core p) {
    int result;

    result = arm_execute_instruction(p);
    if (result)
        arm_exception(p, result);
    return result;
}
