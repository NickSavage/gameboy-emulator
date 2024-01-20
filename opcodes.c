#include <stdio.h>
#include "cpu.h"

static const int REG_B = 0;
static const int REG_C = 1;
static const int REG_D = 2;
static const int REG_E = 3;
static const int REG_H = 4;
static const int REG_L = 5;
static const int REG_F = 6;
static const int REG_A = 7;

static const int FLAG_Z = 7;
static const int FLAG_N = 6;
static const int FLAG_H = 5;
static const int FLAG_C = 4;

static const int COND_NZ = 0;
static const int COND_Z = 1;
static const int COND_NC = 2;
static const int COND_C = 3;

void set_z_flag(struct CPU *cpu) {
    if (cpu->regs[REG_A] == 0) {
	cpu->regs[REG_F] |= 1 << FLAG_Z;
    } else {
	cpu->regs[REG_F] |= 0 << FLAG_Z;
    }
}
void load_reg(struct CPU *cpu, unsigned char reg, unsigned char amount) {
    cpu->regs[reg] = amount;
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
