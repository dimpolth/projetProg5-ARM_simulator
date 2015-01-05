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
CLem
Contact: Guillaume.Huard@imag.fr
         ENSIMAG - Laboratoire LIG
         51 avenue Jean Kuntzmann
         38330 Montbonnot Saint-Martin
*/
#include "arm_core.h"
#include "no_trace_location.h"
#include "arm_constants.h"
#include "arm_exception.h"
#include "util.h"
#include "trace.h"
#include <stdlib.h>

struct arm_core_data {
    int32_t registers_storage[31];
    int32_t *registers[32][16];
    int32_t spsr[32];
    int32_t cpsr;
    uint32_t cycle_count;
    memory mem;
};

arm_core arm_create(memory mem) {
    arm_core p;
    int i, j;

    p = malloc(sizeof(struct arm_core_data));
    if (p) {
        p->mem = mem;
        /* Initialisation of registers tables */
        /* We start with common registers for all modes */
        /* It's quite overkill using that much pointers, but more convenient */
        for (i=0; i<32; i++)
            for (j=0; j<16; j++)
                p->registers[i][j] = &p->registers_storage[j];
        /* Then we point to specific version for each relevant mode */
        p->registers[SVC][13] = &p->registers_storage[16];
        p->registers[SVC][14] = &p->registers_storage[17];
        p->registers[ABT][13] = &p->registers_storage[18];
        p->registers[ABT][14] = &p->registers_storage[19];
        p->registers[UND][13] = &p->registers_storage[20];
        p->registers[UND][14] = &p->registers_storage[21];
        p->registers[IRQ][13] = &p->registers_storage[22];
        p->registers[IRQ][14] = &p->registers_storage[23];
        for (j=8; j<15; j++)
             p->registers[FIQ][j] = &p->registers_storage[j+16];
        arm_exception(p, RESET);
        p->cycle_count = 0;
    }
    return p;
}

void arm_destroy(arm_core p) {
    free(p);
}

int arm_current_mode_has_spsr(arm_core p) {
    uint8_t mode = p->cpsr & 0x1F;
    return (mode != USR) && (mode != SYS);
}

int arm_in_a_privileged_mode(arm_core p) {
    uint8_t mode = p->cpsr & 0x1F;
    return mode != USR;
}

uint32_t arm_get_cycle_count(arm_core p) {
    return p->cycle_count;
}

/* In this implementation, the program counter is incremented during the fetch.
 * Thus, to meet the specification (see manual A2-9), we add 4 whenever the
 * value of the pc is read, so that instructions read their own address + 8 when
 * reading the pc.
 */
uint32_t arm_read_register(arm_core p, uint8_t reg) {
    uint8_t mode = p->cpsr & 0x1f;
    uint32_t value = *p->registers[mode][reg];
    if (reg == 15) {
        value += 4;
        value &= 0xFFFFFFFD;
    }
    trace_register(p->cycle_count, READ, reg, mode, value);
    return value;
}

uint32_t arm_read_usr_register(arm_core p, uint8_t reg) {
    uint32_t value = p->registers_storage[reg];
    if (reg == 15) {
        value += 4;
        value &= 0xFFFFFFFD;
    }
    trace_register(p->cycle_count, READ, reg, USR, value);
    return value;
}

uint32_t arm_read_cpsr(arm_core p) {
    uint32_t value;
    value = p->cpsr;
    trace_register(p->cycle_count, READ, CPSR, 0, value);
    return value;
}

uint32_t arm_read_spsr(arm_core p) {
    uint8_t mode = p->cpsr & 0x1f;
    uint32_t value = p->spsr[mode];
    trace_register(p->cycle_count, READ, SPSR, mode, value);
    return value;
}

void arm_write_register(arm_core p, uint8_t reg, uint32_t value) {
    uint8_t mode = p->cpsr & 0x1f;
    *p->registers[mode][reg] = value;
    trace_register(p->cycle_count, WRITE, reg, mode, value);
}

void arm_write_usr_register(arm_core p, uint8_t reg, uint32_t value) {
    p->registers_storage[reg] = value;
    trace_register(p->cycle_count, WRITE, reg, USR, value);
}

void arm_write_cpsr(arm_core p, uint32_t value) {
    p->cpsr = value;
    trace_register(p->cycle_count, WRITE, CPSR, 0, value);
}

void arm_write_spsr(arm_core p, uint32_t value) {
    uint8_t mode = p->cpsr & 0x1f;
    p->spsr[mode] = value;
    trace_register(p->cycle_count, WRITE, SPSR, mode, value);
}

int arm_fetch(arm_core p, uint32_t *value) {
    int result;
    uint32_t address;

    p->cycle_count++;
    address = arm_read_register(p, 15) - 4;
    result = memory_read_word(p->mem, get_bit(p->cpsr, 9), address, value);
    trace_memory(p->cycle_count, READ, 4, OPCODE_FETCH, address, *value);
    arm_write_register(p, 15, address + 4);
    return result;
}

int arm_read_byte(arm_core p, uint32_t address, uint8_t *value) {
    int result;

    result = memory_read_byte(p->mem, address, value);
    trace_memory(p->cycle_count, READ, 1, OTHER_ACCESS, address, *value);
    return result;
}

/* Data access endianess should comply with bit 9 of cpsr (E), see ARM
 * manual A4-129
 */
int arm_read_half(arm_core p, uint32_t address, uint16_t *value) {
    int result;

    result = memory_read_half(p->mem, get_bit(p->cpsr, 9), address, value);
    trace_memory(p->cycle_count, READ, 2, OTHER_ACCESS, address, *value);
    return result;
}

int arm_read_word(arm_core p, uint32_t address, uint32_t *value) {
    int result;

    result = memory_read_word(p->mem, get_bit(p->cpsr, 9), address, value);
    trace_memory(p->cycle_count, READ, 4, OTHER_ACCESS, address, *value);
    return result;
}

int arm_write_byte(arm_core p, uint32_t address, uint8_t value) {
    int result;

    result = memory_write_byte(p->mem, address, value);
    trace_memory(p->cycle_count, WRITE, 1, OTHER_ACCESS, address, value);
    return result;
}

int arm_write_half(arm_core p, uint32_t address, uint16_t value) {
    int result;

    result = memory_write_half(p->mem, get_bit(p->cpsr, 9), address, value);
    trace_memory(p->cycle_count, WRITE, 2, OTHER_ACCESS, address, value);
    return result;
}

int arm_write_word(arm_core p, uint32_t address, uint32_t value) {
    int result;

    result = memory_write_word(p->mem, get_bit(p->cpsr, 9), address, value);
    trace_memory(p->cycle_count, WRITE, 4, OTHER_ACCESS, address, value);
    return result;
}

void arm_print_state(arm_core p, FILE *out) {
    int mode, reg, count;

    for (mode = 0; mode < 32; mode++) {
        if (arm_get_mode_name(mode)) {
            if (mode != SYS)
                fprintf(out, "%s:", arm_get_mode_name(mode));
            count = 0;
            for (reg=0; reg<16; reg++) {
                if (mode == USR) {
                        if ((reg > 0) && (reg%5 == 0))
                            fprintf(out, "\n    ");
                        fprintf(out, "   %3s=%08X", arm_get_register_name(reg),
                                p->registers_storage[reg]);
                } else if ((p->registers[mode][reg] - p->registers_storage)
                           > 15) {
                        if ((count > 0) && (count%5 == 0))
                            fprintf(out, "\n    ");
                        count++;
                        fprintf(out, "   %3s=%08X", arm_get_register_name(reg),
                                *p->registers[mode][reg]);
                }
            }
            if (mode == USR)
                fprintf(out, "  CPSR=%08X", p->cpsr);
            switch (mode) {
              case USR:
              case FIQ:
              case SVC:
              case UND:
                fprintf(out, "\n");
                break;
              case IRQ:
              case ABT:
                fprintf(out, "          ");
            }
        }
    }
}
