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
	uint8_t numRm, shiftType, shift_imm;
	uint32_t rm, shifter_operand;
	uint32_t cpsr = arm_read_cpsr(p);
	int c, shifter_carry_out;
	c = get_bit(cpsr, C);
	shifter_operand = 0;
	shifter_carry_out = 0;
	numRm = get_bits(ins, 3, 0);
	rm = arm_read_register(p, numRm);
	shiftType = get_bits(ins, 6, 4);
	if(get_bit(ins, 4) == 0)
		shift_imm = get_bits(ins, 11, 7);
	else {
		uint8_t numRs = get_bits(ins, 11, 8);
		shift_imm = get_bits(arm_read_register(p, numRs), 7, 0);
	}
	switch(shiftType) {
		case 0 : //Immediat LSL
			if(shift_imm == 0) {
				shifter_operand = rm;
				shifter_carry_out = c;
			}
			else {
				shifter_operand = rm << shift_imm;
				shifter_carry_out = get_bit(rm, 32-shift_imm);
			}
			break;
		case 1 : //Register LSL
			if(shift_imm == 0) {
				shifter_operand = rm;
				shifter_carry_out = c;
			}
			else if(shift_imm < 32){
				shifter_operand = rm << shift_imm;
				shifter_carry_out = get_bit(rm, 32-shift_imm);
			}
			else if(shift_imm == 32) {
				shifter_operand = 0;
				shifter_carry_out = get_bit(rm, 0);
			}
			else {
				shifter_operand = 0;
				shifter_carry_out = 0;
			}
			break;
		case 2 : //Immediat LSR
			if(shift_imm == 0) {
					shifter_operand = 0;
					shifter_carry_out = get_bit(rm, 31);
			}
			else {
				shifter_operand = rm >> shift_imm;
				shifter_carry_out = get_bit(rm,shift_imm - 1);
			}
			break;
		case 3 : //Register LSR
			if(shift_imm == 0) {
				shifter_operand = rm;
				shifter_carry_out = c;
			}
			else if(shift_imm < 32){
				shifter_operand = rm >> shift_imm;
				shifter_carry_out = get_bit(rm, shift_imm-1);
			}
			else if(shift_imm == 32) {
				shifter_operand = 0;
				shifter_carry_out = get_bit(rm, 31);
			}
			else {
				shifter_operand = 0;
				shifter_carry_out = 0;
			}
			break;
		case 4 : //Immediat ASR
			if(shift_imm == 0) {
				if(get_bit(rm, 31) == 0) {
					shifter_operand = 0;
					shifter_carry_out = get_bit(rm, 31);
				}
				else {
					shifter_operand = 0xFFFFFFFF;
					shifter_carry_out = get_bit(rm, 31);
				}
			}
			else {
				shifter_operand = asr(rm, shift_imm);
				shifter_carry_out = get_bit(rm,shift_imm - 1);
			}
			break;
		case 5 : //Register ASR
			if(shift_imm == 0) {
				shifter_operand = rm;
				shifter_carry_out = c;
			}
			else if(shift_imm < 32){
				shifter_operand = asr(rm, shift_imm);
				shifter_carry_out = get_bit(rm, shift_imm-1);
			}
			else {
				if(get_bit(rm, 31) == 0) {
					shifter_operand = 0;
					shifter_carry_out = get_bit(rm, 31);
				}
				else {
					shifter_operand = 0xFFFFFFFF;
					shifter_carry_out = get_bit(rm, 31);
				}
			}
			break;
		case 6 :
			if(shift_imm == 0) {	//RRX
				shifter_operand = (c << 31) | (rm >> 1);
				shifter_carry_out = get_bit(rm, 0);
			}
			else { //Immediat ROR
				shifter_operand = ror(shift_imm, rm);
				shifter_carry_out = get_bit(shifter_operand, shift_imm-1);
			}
			break;
		case 7 : //Register ROR
			if(shift_imm == 0) {
				shifter_operand = rm;
				shifter_carry_out = c;
			}
			else if(get_bits(shift_imm, 4, 0) == 0) {
				shifter_operand = rm;
				shifter_carry_out = get_bit(rm, 31);
			}
			else {
				shift_imm = get_bits(shift_imm, 4, 0);
				shifter_operand = ror(shift_imm, rm);
				shifter_carry_out = get_bit(shifter_operand, shift_imm -1);
			}
			break;
	}
	opChoice(p, ins, cpsr, shifter_operand, shifter_carry_out);
	return 0;
}

int arm_data_processing_immediate(arm_core p, uint32_t ins) {
	uint32_t shifter_operand;
	uint8_t immed_8;
	uint32_t cpsr = arm_read_cpsr(p);
	int c, shifter_carry_out, rotate_imm;
	c = get_bit(cpsr, C);
	rotate_imm = get_bits(ins, 11, 8);
	immed_8 = get_bits(ins, 8, 0);
	shifter_operand = ror(2*rotate_imm, immed_8);
	if(rotate_imm == 0)
		shifter_carry_out = c;
	else
		shifter_carry_out = get_bit(ins, 31);
	opChoice(p, ins, cpsr, shifter_operand, shifter_carry_out);
	return 0;
}

void opChoice(arm_core p, uint32_t ins, uint32_t cpsr, uint32_t shifter_operand, int shifter_carry_out) {
	uint8_t opcode, numRn, numRd;
	uint32_t res;
	uint32_t rn;
	int n, z, c, v, s;
	numRn = get_bits(ins, 19, 16);
	numRd = get_bits(ins, 15, 12);
	rn = arm_read_register(p, numRn);
	opcode = get_bits(ins, 24, 21);
	s = get_bit(ins, 20);
	n = get_bit(cpsr, N);
	z = get_bit(cpsr, Z);
	c = get_bit(cpsr, C);
	v = get_bit(cpsr, V);
	switch (opcode) {
	case 0:	//AND
		res = rn & shifter_operand;
		if(s == 1 && numRd == 15) {
			if(arm_current_mode_has_spsr(p)) {
				cpsr = arm_read_spsr(p);
				arm_write_cpsr(p, cpsr);
				n = get_bit(cpsr, N);
				z = get_bit(cpsr, Z);
				c = get_bit(cpsr, C);
				v = get_bit(cpsr, V);
			}
		}
		else if(s == 1) {
			n = get_bit(res, 31);
			if(res == 0)
				z = 1;
			else
				z = 0;
			c = shifter_carry_out;
		}
		break;
	case 1:	//EOR
		res = rn ^ shifter_operand;
		if(s == 1 && numRd == 15) {
			if(arm_current_mode_has_spsr(p)) {
				cpsr = arm_read_spsr(p);
				arm_write_cpsr(p, cpsr);
				n = get_bit(cpsr, N);
				z = get_bit(cpsr, Z);
				c = get_bit(cpsr, C);
				v = get_bit(cpsr, V);
			}
		}
		else if(s == 1) {
			n = get_bit(res, 31);
			if(res == 0)
				z = 1;
				else
					z = 0;
				c = shifter_carry_out;
			}
			break;
		case 2:	//SUB
			res = rn - shifter_operand;
			if(s == 1 && numRd == 15) {
				if(arm_current_mode_has_spsr(p)) {
				cpsr = arm_read_spsr(p);
				arm_write_cpsr(p, cpsr);
				n = get_bit(cpsr, N);
				z = get_bit(cpsr, Z);
				c = get_bit(cpsr, C);
				v = get_bit(cpsr, V);
			}
			}
			else if(s == 1) {
				n = get_bit(res, 31);
				if(res == 0)
					z = 1;
				else
					z = 0;
				if(rn < shifter_operand)
					c = 0;
				else
					c = 1;
				if(get_bit(rn, 31) == get_bit(shifter_operand, 31))
					v = 0;
				else
					v = 1;
			}
			break;
		case 3:	//RSB
			res = shifter_operand - rn;
			if(s == 1 && numRd == 15) {
				if(arm_current_mode_has_spsr(p)) {
				cpsr = arm_read_spsr(p);
				arm_write_cpsr(p, cpsr);
				n = get_bit(cpsr, N);
				z = get_bit(cpsr, Z);
				c = get_bit(cpsr, C);
				v = get_bit(cpsr, V);
			}
			}
			else if(s == 1) {
				n = get_bit(res, 31);
				if(res == 0)
					z = 1;
				else
					z = 0;
				if(rn > shifter_operand)
					c = 0;
				else
					c = 1;
				if(get_bit(rn, 31) == get_bit(shifter_operand, 31))
					v = 1;
				else
					v = 0;
			}
			break;
		case 4:	//ADD
			res = rn + shifter_operand;
			if(s == 1 && numRd == 15) {
				if(arm_current_mode_has_spsr(p)) {
				cpsr = arm_read_spsr(p);
				arm_write_cpsr(p, cpsr);
				n = get_bit(cpsr, N);
				z = get_bit(cpsr, Z);
				c = get_bit(cpsr, C);
				v = get_bit(cpsr, V);
			}
			}
			else if(s == 1) {
				n = get_bit(res, 31);
				if(res == 0)
					z = 1;
				else
					z = 0;
				if (get_bit(rn, 31) == get_bit(shifter_operand, 31) && get_bit(res, 31) != get_bit(rn, 31))
					v = 1;
				else
					v = 0;
				int it; c = 0;
				for(it = 0; it<32; it++) {
					c = (c + get_bit(shifter_operand, it) + get_bit(rn, it)) >> 1;
				}
			}
			break;
		case 5:	//ADC
			res = rn + shifter_operand + c;
			if(s == 1 && numRd == 15) {
				if(arm_current_mode_has_spsr(p)) {
				cpsr = arm_read_spsr(p);
				arm_write_cpsr(p, cpsr);
				n = get_bit(cpsr, N);
				z = get_bit(cpsr, Z);
				c = get_bit(cpsr, C);
				v = get_bit(cpsr, V);
			}
			}
			else if(s == 1) {
				n = get_bit(res, 31);
				if(res == 0)
					z = 1;
				else
					z = 0;
				if (get_bit(rn, 31) == get_bit(shifter_operand, 31) && get_bit(res, 31) != get_bit(rn, 31))
					v = 1;
				else
					v = 0;
				int it; c = 0;
				for(it = 0; it<32; it++) {
					c = (c + get_bit(shifter_operand, it) + get_bit(rn, it)) >> 1;
				}
			}
			break;
		case 6:	//SBC
			res = rn - shifter_operand - ~c;
			if(s == 1 && numRd == 15) {
				if(arm_current_mode_has_spsr(p)) {
				cpsr = arm_read_spsr(p);
				arm_write_cpsr(p, cpsr);
				n = get_bit(cpsr, N);
				z = get_bit(cpsr, Z);
				c = get_bit(cpsr, C);
				v = get_bit(cpsr, V);
			}
			}
			else if(s == 1) {
				n = get_bit(res, 31);
				if(res == 0)
					z = 1;
				else
					z = 0;
				if(rn < shifter_operand)
					c = 0;
				else
					c = 1;
				if(get_bit(rn, 31) == get_bit(shifter_operand, 31))
					v = 0;
				else
					v = 1;
			}
			break;
		case 7:	//RSC
			res = shifter_operand - rn - ~c;
			if(s == 1 && numRd == 15) {
				if(arm_current_mode_has_spsr(p)) {
				cpsr = arm_read_spsr(p);
				arm_write_cpsr(p, cpsr);
				n = get_bit(cpsr, N);
				z = get_bit(cpsr, Z);
				c = get_bit(cpsr, C);
				v = get_bit(cpsr, V);
			}
			}
			else if(s == 1) {
				n = get_bit(res, 31);
				if(res == 0)
					z = 1;
				else
					z = 0;
				if(rn > shifter_operand)
					c = 0;
				else
					c = 1;
				if(get_bit(rn, 31) == get_bit(shifter_operand, 31))
					v = 1;
				else
					v = 0;
			}
			break;
		case 8:	//TST
			res = rn & shifter_operand;
			n = get_bit(res, 31);
			if(res == 0)
				z = 1;
			else
				z = 0;
			c = shifter_carry_out;
			break;
		case 9:	//TEQ
			res = rn ^ shifter_operand;
			n = get_bit(res, 31);
			if(res == 0)
				z = 1;
			else
				z = 0;
			c = shifter_carry_out;
			break;
		case 10:	//CMP
			res = rn - shifter_operand;
			n = get_bit(res, 31);
			if(res == 0)
				z = 1;
			else
				z = 0;
			if(rn < shifter_operand)
				c = 0;
			else
				c = 1;
			if(get_bit(rn, 31) == get_bit(shifter_operand, 31))
				v = 0;
			else
				v = 1;
			break;
		case 11:	//CMN
			res = rn + shifter_operand;
			n = get_bit(res, 31);
			if(res == 0)
				z = 1;
			else
				z = 0;
			if (get_bit(rn, 31) == get_bit(shifter_operand, 31) && get_bit(res, 31) != get_bit(rn, 31))
					v = 1;
				else
					v = 0;
				int it; c = 0;
				for(it = 0; it<32; it++) {
					c = (c + get_bit(shifter_operand, it) + get_bit(rn, it)) >> 1;
				}
			break;
		case 12:	//ORR
			res = rn | shifter_operand;
			if(s == 1 && numRd == 15) {
				if(arm_current_mode_has_spsr(p)) {
				cpsr = arm_read_spsr(p);
				arm_write_cpsr(p, cpsr);
				n = get_bit(cpsr, N);
				z = get_bit(cpsr, Z);
				c = get_bit(cpsr, C);
				v = get_bit(cpsr, V);
			}
			}
			else if(s == 1) {
				n = get_bit(res, 31);
				if(res == 0)
					z = 1;
				else
					z = 0;
				c = shifter_carry_out;
			}
			break;
		case 13:	//MOV
			res = shifter_operand;
			if(s == 1 && numRd == 15) {
				if(arm_current_mode_has_spsr(p)) {
				cpsr = arm_read_spsr(p);
				arm_write_cpsr(p, cpsr);
				n = get_bit(cpsr, N);
				z = get_bit(cpsr, Z);
				c = get_bit(cpsr, C);
				v = get_bit(cpsr, V);
			}
			}
			else if(s == 1) {
				n = get_bit(res, 31);
				if(res == 0)
					z = 1;
				else
					z = 0;
				c = shifter_carry_out;
			}
			break;
		case 14:	//BIC
			res = rn & ~shifter_operand;
			if(s == 1 && numRd == 15) {
				if(arm_current_mode_has_spsr(p)) {
				cpsr = arm_read_spsr(p);
				arm_write_cpsr(p, cpsr);
				n = get_bit(cpsr, N);
				z = get_bit(cpsr, Z);
				c = get_bit(cpsr, C);
				v = get_bit(cpsr, V);
			}
			}
			else if(s == 1) {
				n = get_bit(res, 31);
				if(res == 0)
					z = 1;
				else
					z = 0;
				c = shifter_carry_out;
			}
			break;
		case 15:	//MVN
			res = ~shifter_operand;
			if(s == 1 && numRd == 15) {
				if(arm_current_mode_has_spsr(p)) {
				cpsr = arm_read_spsr(p);
				arm_write_cpsr(p, cpsr);
				n = get_bit(cpsr, N);
				z = get_bit(cpsr, Z);
				c = get_bit(cpsr, C);
				v = get_bit(cpsr, V);
			}
			}
			else if(s == 1) {
				n = get_bit(res, 31);
				if(res == 0)
					z = 1;
				else
					z = 0;
				c = shifter_carry_out;
			}
			break;
	}
	if(opcode < 8 || opcode > 11)
		arm_write_register(p, numRd, res);
	if(n == 0)
		cpsr = clr_bit(cpsr, N);
	else
		cpsr = set_bit(cpsr, N);
	if(z == 0)
		cpsr = clr_bit(cpsr, Z);
	else
		cpsr = set_bit(cpsr, Z);
	if(v == 0)
		cpsr = clr_bit(cpsr, V);
	else
		cpsr = set_bit(cpsr, V);
	if(c == 0)
		cpsr = clr_bit(cpsr, C);
	else
		cpsr = set_bit(cpsr, C);
	arm_write_cpsr(p, cpsr);
}
