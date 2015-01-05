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
#include <stdlib.h>
#include "memory.h"
#include "util.h"

struct memory_data {
    int8_t *address;
    size_t size;
};

memory memory_create(size_t size) {
    memory mem;

    mem = malloc(sizeof(struct memory_data));
    if (mem) {
        mem->address = malloc(size);
        if (!mem->address) {
            free(mem);
            mem = NULL;
        }
    }
    if (mem) {
        mem->size = size;
    }
    return mem;
}

size_t memory_get_size(memory mem) {
    return mem->size;
}

void memory_destroy(memory mem) {
    free(mem->address);
    free(mem);
}

int memory_read_byte(memory mem, uint32_t address, uint8_t *value) {
    
    if (address > mem->size)
	return -1;
    int8_t * ad;
    ad = mem->address + address;
    *value = *ad;
    return 0;
}

int memory_read_half(memory mem, int be, uint32_t address, uint16_t *value) {
    
    if (address+1 > mem->size)
	return -1;
    int8_t * ad;
    int16_t val = 0;
    int i;
    ad = mem->address + address;

    if (be == 0) {
       for (i=0;i<=1;i++) {
          val = val | *(ad+i) << (i*8);
       }
    } else {
       for (i=1;i>=0;i--) {
          val = val | *(ad+i) << ((1-i)*8);
       }
    }
    *value = val;
    return 0;
}

int memory_read_word(memory mem, int be, uint32_t address, uint32_t *value) {
    
    if (address+3 > mem->size)
	return -1;
    int8_t * ad;
    int32_t val = 0;
    int i;
    ad = mem->address + address;

    if (be == 0) {
       for (i=0;i<=3;i++) {
          val = val | *(ad+i) << (i*8);
       }
    } else {
       for (i=3;i>=0;i--) {
          val = val | *(ad+i) << ((3-i)*8);
       }
    }
    *value = val;
    return 0;
}

int memory_write_byte(memory mem, uint32_t address, uint8_t value) {
    
    if (address > mem->size)
	return -1;
    int8_t * ad;
    ad = mem->address + address;
    *ad = value;
    return 0;
}

int memory_write_half(memory mem, int be, uint32_t address, uint16_t value) {
    
    if (address+1 > mem->size)
	return -1;
    int8_t * ad;
    int i;
    ad = mem->address + address;

    if (be == 0) {
       for (i=0;i<=1;i++) {
          *(ad+i) = (value & 255 << (i*8)) >> (i*8);
       }
    } else {
       for (i=1;i>=0;i--) {
          *(ad+i) = (value & 255 << ((1-i)*8)) >> ((1-i)*8);
       }
    }
    return 0;
}

int memory_write_word(memory mem, int be, uint32_t address, uint32_t value) {
    
    if (address+3 > mem->size)
	return -1;
    int8_t * ad;
    int i;
    ad = mem->address + address;

    if (be == 0) {
       for (i=0;i<=3;i++) {
          *(ad+i) = (value & 255 << (i*8)) >> (i*8);
       }
    } else {
       for (i=3;i>=0;i--) {
          *(ad+i) = (value & 255 << ((3-i)*8)) >> ((3-i)*8);
       }
    }
    return 0;
}
