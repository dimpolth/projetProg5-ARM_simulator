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
#include "arm_data_processing.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "arm_branch_other.h"
#include "util.h"
#include "debug.h"

/* Decoding functions for different classes of instructions */
int arm_data_processing_shift(arm_core p, uint32_t ins) {
	uint8_t opcode, s, numRn, numRd, shiftOp, numRm, shiftDirection;
	uint32_t rn, res, rm, shiftValue, valShifted;
	uint32_t cpsr = arm_read_cpsr(p) >> 28;
	int n, z, v, c, carryOut;
	n = (cpsr & 8) >> 3;
	z = (cpsr & 4) >> 2;
	c = (cpsr & 2) >> 1;
	v = (cpsr & 1);
	opcode = get_bits(ins, 24, 21);
	s = get_bit(ins, 20);
	numRn = get_bits(ins, 19, 16);
	numRd = get_bits(ins, 15, 12);
	numRm = get_bits(ins, 3, 0);
	rn = arm_read_register(p, numRn);
	rm = arm_read_register(p, numRm);
	shiftOp = get_bit(ins, 4); //0 si opérande immadiate, 1 si c'est un registre
	if(shiftOp == 0) {
		shiftValue = get_bits(ins, 11, 7);
		if(shiftValue == 0) {
			valShifted = rm;
			carryOut = c;
		}
		else {
			shiftDirection = get_bit(ins, 5)
			if(shiftDirection == LSL) {
				shiftedValue = rm << shiftValue;
				carryOut = get_bit(rm, 32 - shiftValue);
			}
			else {
				shiftedValue = rm >> shiftValue;
				carryOut = get_bit(rm, shiftValue - 1);
				//TODO
			}
		}
	}
	else {
		uint8_t numRs = get_bits(ins, 11, 8);
		shiftValue = get_bits(arm_read_register(p, numRs), 7, 0);
			if(shiftValue == 0) {
				valShifted = rm;
				carryOut = c;
			}
			else {
				//TODO
			}
	}
	
	switch (opcode) {
		case 0:	//AND
			res = rn & valShifted;
			break;
		case 1:	//EOR
			res = rn ^ valShifted;
			break;
		case 2:	//SUB
			res = rn - valShifted;
			break;
		case 3:	//RSB
			res = valShifted - rn;
			break;
		case 4:	//ADD
			res = rn + valShifted;
			break;
		case 5:	//ADC
			res = rn + valShifted + c;
			n = get_bit(res, 31);
			if(res == 0)
				z = 1;
			else
				z = 0;
			c = 
			v = 
			break;
		case 6:	//SBC
			res = rn - valShifted - ~c;
			break;
		case 7:	//RSC
			res = valShifted - rn - ~c;
			break;
		case 8:	//TST
			res = rn & valShifted;
			break;
		case 9:	//TEQ
			res = rn ^ valShifted;
			break;
		case 10:	//CMP
			res = rn - valShifted;
			break;
		case 11:	//CMN
			res = rn + valShifted;
			break;
		case 12:	//ORR
			res = rn | valShifted;
			break;
		case 13:	//MOV
			res = valShifted;
			break;
		case 14:	//BIC
			res = rn & ~valShifted;
			break;
		case 15:	//MVN
			res = ~valShifted;
			break;
	}
	if(opcode < 8 || opcode > 11)
		arm_write_register(p, numRd, res);
	return UNDEFINED_INSTRUCTION;
}

int arm_data_processing_immediate_msr(arm_core p, uint32_t ins) {
    return UNDEFINED_INSTRUCTION;
}
