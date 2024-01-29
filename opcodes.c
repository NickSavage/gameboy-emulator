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
    } else if (reg == 3) {
	cpu->sp = (high << 8) + low;
	
    }else {
	printf("error: load_reg_16 out of bounds, %d\n", reg);
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

void add_16(struct CPU *cpu, uint8_t reg, uint16_t amount) {
    int total; 
    
    if (reg == 0) {
	total = (cpu->regs[REG_B] << 8) + cpu->regs[REG_C] + amount;
	cpu->regs[REG_B] = (total >> 8) & 0xFF;
	cpu->regs[REG_C] = total & 0xFF;
	
    } else if (reg == 1) {
	total = (cpu->regs[REG_D] << 8) + cpu->regs[REG_E] + amount;
	cpu->regs[REG_D] = (total >> 8) & 0xFF;
	cpu->regs[REG_E] = total & 0xFF;
	
    } else if (reg == 2) {
	total = (cpu->regs[REG_H] << 8) + cpu->regs[REG_L] + amount;
	cpu->regs[REG_H] = (total >> 8) & 0xFF;
	cpu->regs[REG_L] = total & 0xFF;
    } else if (reg == 3) {
	printf("not implemented");
	exit(0);
    }else {
	exit(0);
	printf("error: load_reg_16 out of bounds, %d\n", reg);
    }
    
}

void decrement_8(struct CPU *cpu, uint8_t reg) {
    cpu->regs[reg] -= 1;

    if (cpu->regs[reg] == 0) {
	cpu->regs[REG_F] |= 1 << FLAG_Z;
    } else {
        cpu->regs[REG_F] &= ~(1 << FLAG_Z);
    }
    set_n_flag(cpu, 1);
    
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

void push(struct CPU *cpu, uint8_t reg) {
    if (reg == 0) {
	cpu->memory[cpu->sp - 1] = cpu->regs[REG_B];
	cpu->memory[cpu->sp - 2] = cpu->regs[REG_C];
    } else if (reg == 1) {
	cpu->memory[cpu->sp - 1] = cpu->regs[REG_D];
	cpu->memory[cpu->sp - 2] = cpu->regs[REG_E];
	
    } else if (reg == 2) {
	cpu->memory[cpu->sp - 1] = cpu->regs[REG_H];
	cpu->memory[cpu->sp - 2] = cpu->regs[REG_L];
    } else {
	printf("something is out of bounds");
    }
    cpu->sp -= 2;
}
void pop(struct CPU *cpu, uint8_t reg) {
    if (reg == 0) {
	cpu->regs[REG_C] = cpu->memory[cpu->sp];
	cpu->regs[REG_B] = cpu->memory[cpu->sp + 1];
    } else if (reg == 1) {
	cpu->regs[REG_E] = cpu->memory[cpu->sp];
	cpu->regs[REG_D] = cpu->memory[cpu->sp + 1];
    } else if (reg == 2) {
	cpu->regs[REG_L] = cpu->memory[cpu->sp];
	cpu->regs[REG_H] = cpu->memory[cpu->sp + 1];
    } else {
	printf("something is out of bounds");
    }
    cpu->sp += 2;
}
void call(struct CPU *cpu, uint8_t high, uint8_t low) {

    cpu->memory[cpu->sp - 1] = (cpu->pc >> 8) & 0xFF;
    cpu->memory[cpu->sp - 2] = cpu->pc & 0xFF;
    cpu->sp -= 2;
    cpu->pc = (high << 8) + low;
}

void ret_function(struct CPU *cpu) {
    uint16_t addr = (cpu->memory[cpu->sp + 1] << 8) + cpu->memory[cpu->sp];
    cpu->pc = addr;
    cpu->sp += 2;
}

uint16_t interleave_tile_pixel(uint8_t low, uint8_t high, uint8_t index) {
    uint16_t result = 0;

    result = (((high >> index) & 0x1) << 1) + ((low >> index) & 0x1);
    return result;
}
uint16_t interleave_tile(uint8_t low, uint8_t high) {
    uint16_t result = 0;

    for (int i = 0; i < 8; ++i) {
        result |= (high & (1 << i)) << (i + 1);
        result |= (low & (1 << i)) << i;
    }
    printf("%d",result);
    return result;
}
