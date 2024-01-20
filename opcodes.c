#include <stdio.h>
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
	cpu->regs[REG_F] |= 0 << FLAG_Z;
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

void set_mem(struct CPU *cpu, int16_t addr, int8_t amount) {
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

void xor(struct CPU *cpu, unsigned char reg) {
    cpu->regs[REG_A] = cpu->regs[REG_A] ^ cpu->regs[reg];

    set_z_flag(cpu);
    set_n_flag(cpu, 0);
    set_c_flag(cpu, 0);
    set_h_flag(cpu, 0);
}
