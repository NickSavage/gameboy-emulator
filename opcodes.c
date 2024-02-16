#include <stdio.h>
#include <stdint.h>
#include "cpu.h"


void printByteAsBinary(unsigned char byte) {
    for (int i = 7; i >= 0; i--) {
        putchar((byte & (1 << i)) ? '1' : '0');
    }
}

void request_vblank_int(struct CPU *cpu) {
    cpu->memory[0xffff] |= 1 << 0;
}

void set_z_flag(struct CPU *cpu, uint8_t reg) {
    if (cpu->regs[reg] == 0) {
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
    if (bit > 0) {  
        cpu->regs[REG_F] |= (1 << FLAG_C);
    } else if (bit == 0) {
	cpu->regs[REG_F] &= ~(1 << FLAG_C);
    } else {
	printf("set_c_flag given not 1 or 0");
	exit(0);
    }
}
void set_h_flag(struct CPU *cpu, int bit) {
    if (bit > 0) {  
        cpu->regs[REG_F] |= (1 << FLAG_H);
    } else if (bit == 0) {
	cpu->regs[REG_F] &= ~(1 << FLAG_H);
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

void reset_bit(struct CPU *cpu, uint8_t reg, uint8_t bit) {
    uint8_t bitmask = 0xFF ^ (1 << bit);
    cpu->regs[reg] &= bitmask;
}
void swap(struct CPU *cpu, uint8_t reg) {
    uint8_t high = cpu->regs[reg] >> 4;
    uint8_t low = cpu->regs[reg] & 0b00001111;

    cpu->regs[reg] = (low << 4) + high;
}

void srl(struct CPU *cpu, uint8_t reg) {
    set_c_flag(cpu, cpu->regs[reg] & 0x01);
    cpu->regs[reg] = cpu->regs[reg] >> 1;
}

void bit(struct CPU *cpu, uint8_t n, uint8_t reg) {

    if (reg == 0b110) {
	// hl
	uint16_t addr = (cpu->memory[REG_H] << 8) + cpu->memory[REG_L];

	if ((cpu->memory[addr] & 1 << n ) > 1) {
	    cpu->regs[REG_F] &= ~(1 << FLAG_Z);
	} else {
	    cpu->regs[REG_F] |= 1 << FLAG_Z;
	}
	
    } else {
	if ((cpu->regs[reg] & 1 << n ) > 1) {
	    cpu->regs[REG_F] &= ~(1 << FLAG_Z);
	} else {
	    cpu->regs[REG_F] |= 1 << FLAG_Z;
	}
	
    }
    
    set_h_flag(cpu, 1);
    set_n_flag(cpu, 0);
}

void cpl(struct CPU *cpu) {
    cpu->regs[REG_A] = ~cpu->regs[REG_A];
    set_n_flag(cpu, 1);
    set_h_flag(cpu, 1);
}

void ccf(struct CPU *cpu) {
    if (get_c_flag(cpu) == 1) {
	set_c_flag(cpu, 0);
    } else {
	set_c_flag(cpu, 1);
    }
}

void rla(struct CPU *cpu) {
    uint8_t temp = cpu->regs[REG_A] & 0b00000001;
    cpu->regs[REG_A] = (cpu->regs[REG_A] >> 1) + (get_c_flag(cpu) << 7);
    set_c_flag(cpu, temp);
}
void daa(struct CPU *cpu) {
    // Extract the lower nibble (4 bits) and the upper nibble (4 bits) of the accumulator
    unsigned char lower_nibble = cpu->regs[REG_A] & 0x0F;
    unsigned char upper_nibble = (cpu->regs[REG_A] >> 4) & 0x0F;

    // Check if the lower nibble is greater than 9 or the half-carry flag is set
    if (lower_nibble > 9 || get_h_flag(cpu)) {
         cpu->regs[REG_A] += 6; // Increment accumulator by 6
	 set_h_flag(cpu, ((int)cpu->regs[REG_A] & 0x0F) < 6); // Update half-carry flag
    }

    // Check if the upper nibble is greater than 9 or the carry flag is set
    if (upper_nibble > 9 || get_c_flag(cpu)) {
	cpu->regs[REG_A] += 0x60;
	set_c_flag(cpu, 1);
    }
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

void add(struct CPU *cpu, uint8_t reg, unsigned char amount) {
    set_h_flag(cpu, ((cpu->regs[reg] & 0x0F) + (amount & 0x0F)) & 0x10);
    cpu->regs[reg] += amount;

    set_z_flag(cpu, reg);

    set_n_flag(cpu, 0);
    
    cpu->regs[REG_F] |= 0 << FLAG_N;

    //    printf("%d, %d", cpu->regs[reg], amount);
    if ((cpu->regs[reg]) < 0) {
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

void adc(struct CPU *cpu, uint8_t amount) {
    uint16_t result = cpu->regs[REG_A] + get_c_flag(cpu) + amount;
    set_z_flag(cpu, REG_A);
    set_c_flag(cpu, (result & 0x100));
    set_h_flag(cpu, ((cpu->regs[REG_A] & 0x0F) + (amount & 0x0F)) & 0x10);

    set_n_flag(cpu, 0);
    cpu->regs[REG_A] = result & 0xFF;
}

void decrement_8(struct CPU *cpu, uint8_t reg) {

    set_h_flag(cpu, ((cpu->regs[reg] & 0x0F) - (1 & 0x0F)) & 0x10);
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
    
    set_z_flag(cpu, REG_A);
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

        cpu->regs[REG_F] &= ~(1 << FLAG_Z);
    }
    cpu->regs[REG_F] |= 1 << FLAG_N;
    if (result < 0) {
	cpu->regs[REG_F] |= 1 << FLAG_C;
    } else {
        cpu->regs[REG_F] &= ~(1 << FLAG_C);
    }
}

void and(struct CPU *cpu, unsigned char amount) {
    set_h_flag(cpu, ((cpu->regs[REG_A] & 0x0F) + (amount & 0x0F)) & 0x10);
    set_n_flag(cpu, 0);
    set_c_flag(cpu, 0);

    cpu->regs[REG_A] = cpu->regs[REG_A] & amount;
    
    set_z_flag(cpu, REG_A);
}

void or_8(struct CPU *cpu, uint8_t amount) {
    
    set_h_flag(cpu, ((cpu->regs[REG_A] & 0x0F) | (amount & 0x0F)) & 0x10);
    cpu->regs[REG_A] = cpu->regs[REG_A] | amount;
    set_z_flag(cpu, REG_A);
    set_n_flag(cpu, 0);
    set_c_flag(cpu, 0);
    
}
void or(struct CPU *cpu, unsigned char reg) {
    or_8(cpu, cpu->regs[reg]);
}


void xor(struct CPU *cpu, unsigned char reg) {
    set_h_flag(cpu, ((cpu->regs[REG_A] & 0x0F) ^ (cpu->regs[reg] & 0x0F)) & 0x10);
    cpu->regs[REG_A] = cpu->regs[REG_A] ^ cpu->regs[reg];

    set_z_flag(cpu, REG_A);
    set_n_flag(cpu, 0);
    set_c_flag(cpu, 0);
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
    } else if (reg == 3) {
	cpu->memory[cpu->sp - 1] = cpu->regs[REG_A];
	cpu->memory[cpu->sp - 2] = cpu->regs[REG_F];
    }

    else {
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
    } else if (reg == 3) {
	cpu->regs[REG_F] = cpu->memory[cpu->sp];
	cpu->regs[REG_A] = cpu->memory[cpu->sp + 1];
    }

    else {
	printf("something is out of bounds");
    }
    cpu->sp += 2;
}
void call(struct CPU *cpu, uint8_t high, uint8_t low) {

    cpu->pc += 2;
    cpu->memory[cpu->sp - 1] = (cpu->pc >> 8) & 0xFF;
    cpu->memory[cpu->sp - 2] = cpu->pc & 0xFF;
    //    printf(" - %x, %x, %x, ret: \n", cpu->sp, cpu->memory[cpu->sp - 1], cpu->memory[cpu-> sp - 2]);
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
    // printf("%d",result);
    return result;
}
