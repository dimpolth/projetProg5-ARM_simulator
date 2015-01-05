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
    /* We only support RESET initially */
    /* Semantics of reset interrupt (ARM manual A2-18) */
    if (exception == RESET) {
        arm_write_cpsr(p, 0x1d3 | Exception_bit_9);
	arm_write_usr_register(p, 15, 0);
    }
}
