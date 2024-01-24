#include <stdio.h>
#include <stdint.h>
#include "cpu.h"


void printByteAsBinary(unsigned char byte) {
    for (int i = 7; i >= 0; i--) {
        putchar((byte & (1 << i)) ? '1' : '0');
    }
}


void set_z_flag(struct CPU *cpu) {
    if (cpu->regs[REG_A] == 0) {
	cpu->regs[REG_F] |= 1 << FLAG_Z;
    } else {
        cpu->regs[REG_F] &= ~(1 << FLAG_Z);
    }
}

void set_n_flag(struct CPU *cpu, int bit) {
    if (bit == 1) {  
        cpu->regs[REG_F] |= (1 << FLAG_N);
    } else if (bit == 0) {
	cpu->regs[REG_F] &= ~(1 << FLAG_N);
    } else {
	printf("set_n_flag given not 1 or 0");
	exit(0);
    }
}

void set_c_flag(struct CPU *cpu, int bit) {
    if (bit == 1) {  
        cpu->regs[REG_F] |= (1 << FLAG_C);
    } else if (bit == 0) {
	cpu->regs[REG_F] &= ~(1 << FLAG_C);
    } else {
	printf("set_c_flag given not 1 or 0");
	exit(0);
    }
}
void set_h_flag(struct CPU *cpu, int bit) {
    if (bit == 1) {  
        cpu->regs[REG_F] |= (1 << FLAG_H);
    } else if (bit == 0) {
	cpu->regs[REG_F] &= ~(1 << FLAG_H);
    } else {
	printf("set_h_flag given not 1 or 0");
	exit(0);
    }
}
int get_z_flag(struct CPU *cpu) {
    int ret = (cpu->regs[REG_F] & 0b10000000) >> FLAG_Z;
    return ret;
}

int get_n_flag(struct CPU *cpu) {
    int ret = (cpu->regs[REG_F] & 0b01000000) >> FLAG_N;
    return ret;
}
int get_c_flag(struct CPU *cpu) {
    int ret = (cpu->regs[REG_F] & 0b00010000) >> FLAG_C;
    return ret;
}
int get_h_flag(struct CPU *cpu) {
    int ret = (cpu->regs[REG_F] & 0b00100000) >> FLAG_H;
    return ret;
}

void load_reg(struct CPU *cpu, unsigned char reg, unsigned char amount) {
    cpu->regs[reg] = amount;
}
void load_reg_16(struct CPU *cpu, unsigned char reg, unsigned char low, unsigned char high) {
    if (reg == 0) {
	cpu->regs[REG_B] = high;
	cpu->regs[REG_C] = low;
	
    } else if (reg == 1) {
	cpu->regs[REG_D] = high;
	cpu->regs[REG_E] = low;
	
    } else if (reg == 2) {
	cpu->regs[REG_H] = high;
	cpu->regs[REG_L] = low;
    } else {
	printf("error: load_reg_16 out of bounds, %d", reg);
    }
    
};

void set_mem(struct CPU *cpu, uint16_t addr, uint8_t amount) {
    cpu->memory[addr] = amount;
}

void add(struct CPU *cpu, unsigned char amount) {
    cpu->regs[REG_A] += amount;

    set_z_flag(cpu);
    
    cpu->regs[REG_F] |= 0 << FLAG_N;
    printf("%d, %d", cpu->regs[REG_A], amount);
    if ((cpu->regs[REG_A]) < 0) {
	cpu->regs[REG_F] |= 1 << FLAG_C;
    } else {
	cpu->regs[REG_F] |= 0 << FLAG_C;
    }
}

void sub(struct CPU *cpu, unsigned char amount) {
    cpu->regs[REG_A] -= amount;
    
    set_z_flag(cpu);
    cpu->regs[REG_F] |= 1 << FLAG_N;
    if ((cpu->regs[REG_A] - amount) < 0) {
	cpu->regs[REG_F] |= 1 << FLAG_C;
    } else {
	cpu->regs[REG_F] |= 0 << FLAG_C;
    }
    
}

void compare(struct CPU *cpu, unsigned char amount) {
    int result = cpu->regs[REG_A] - amount;

    if (result == 0) {
	cpu->regs[REG_F] |= 1 << FLAG_Z;
    } else {
	cpu->regs[REG_F] |= 0 << FLAG_Z;
    }
    cpu->regs[REG_F] |= 1 << FLAG_N;
    if (result < 0) {
	cpu->regs[REG_F] |= 1 << FLAG_C;
    } else {
	cpu->regs[REG_F] |= 0 << FLAG_C;
    }
}

void and(struct CPU *cpu, unsigned char amount) {
    cpu->regs[REG_A]= cpu->regs[REG_A] & amount;
    
    set_z_flag(cpu);
    set_n_flag(cpu, 0);
    set_c_flag(cpu, 0);
    set_h_flag(cpu, 1);
}

void or(struct CPU *cpu, unsigned char reg) {
    cpu->regs[REG_A] = cpu->regs[REG_A] | cpu->regs[reg];
    set_z_flag(cpu);
    set_n_flag(cpu, 0);
    set_c_flag(cpu, 0);
    set_h_flag(cpu, 0);
}

void xor(struct CPU *cpu, unsigned char reg) {
    cpu->regs[REG_A] = cpu->regs[REG_A] ^ cpu->regs[reg];

    set_z_flag(cpu);
    set_n_flag(cpu, 0);
    set_c_flag(cpu, 0);
    set_h_flag(cpu, 0);
}


uint16_t interleave_tile(uint8_t low, uint8_t high) {
    uint16_t result = 0;

    for (int i = 0; i < 8; ++i) {
        result |= (high & (1 << i)) << (i + 1);
        result |= (low & (1 << i)) << i;
    }
    return result;
}

uint16_t* fetch_tile(struct CPU *cpu, uint16_t addr) {
    uint16_t* tile = malloc(8 * sizeof(uint16_t)); // Allocate memory for 8 uint16_t elements
    
    tile[0] = interleave_tile(cpu->memory[addr], cpu->memory[addr + 1]);
    tile[1] = interleave_tile(cpu->memory[addr + 1], cpu->memory[addr + 2]);
    tile[2] = interleave_tile(cpu->memory[addr + 3], cpu->memory[addr + 4]);
    tile[3] = interleave_tile(cpu->memory[addr + 5], cpu->memory[addr + 6]);
    tile[4] = interleave_tile(cpu->memory[addr + 7], cpu->memory[addr + 8]);
    tile[5] = interleave_tile(cpu->memory[addr + 9], cpu->memory[addr + 10]);
    tile[6] = interleave_tile(cpu->memory[addr + 11], cpu->memory[addr + 12]);
    tile[7] = interleave_tile(cpu->memory[addr + 13], cpu->memory[addr + 14]);
    tile[8] = interleave_tile(cpu->memory[addr + 15], cpu->memory[addr + 16]);
    /* for (int i = 0; i < 8; i++) { */
    /* 	printf("%X", tile[i]); */
    /* } */
    /* printf("\n"); */
    return tile;
}
