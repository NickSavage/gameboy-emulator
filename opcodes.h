#include <stdint.h>

#include "cpu.h"

void printByteAsBinary(unsigned char byte);

void set_z_flag(struct CPU *cpu);
void set_n_flag(struct CPU *cpu, int bit);
void set_c_flag(struct CPU *cpu, int bit);
void set_h_flag(struct CPU *cpu, int bit);

int get_z_flag(struct CPU *cpu);
int get_n_flag(struct CPU *cpu);
int get_c_flag(struct CPU *cpu);
int get_h_flag(struct CPU *cpu);

void load_reg(struct CPU *cpu, unsigned char reg, unsigned char amount);
void load_reg_16(struct CPU *cpu, unsigned char reg, unsigned char low, unsigned char high);
void set_mem(struct CPU *cpu, uint16_t addr, uint8_t amount);
void add(struct CPU *cpu, unsigned char amount);
void sub(struct CPU *cpu, unsigned char amount);
void compare(struct CPU *cpu, unsigned char amount);
void and(struct CPU *cpu, unsigned char amount);
void or(struct CPU *cpu, unsigned char reg);
void xor(struct CPU *cpu, unsigned char reg);

uint16_t fetch_tile(struct CPU *cpu, uint16_t addr);
