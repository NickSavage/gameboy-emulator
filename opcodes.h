#include <stdint.h>

#include "cpu.h"

void printByteAsBinary(unsigned char byte);

void request_vblank_int(struct CPU *cpu);

void set_z_flag(struct CPU *cpu, uint8_t reg);
void set_n_flag(struct CPU *cpu, int bit);
void set_c_flag(struct CPU *cpu, int bit);
void set_h_flag(struct CPU *cpu, int bit);

int get_z_flag(struct CPU *cpu);
int get_n_flag(struct CPU *cpu);
int get_c_flag(struct CPU *cpu);
int get_h_flag(struct CPU *cpu);

void reset_bit(struct CPU *cpu, uint8_t reg, uint8_t bit);
void swap(struct CPU *cpu, uint8_t reg);
void srl(struct CPU *cpu, uint8_t reg);
void bit(struct CPU *cpu, uint8_t n, uint8_t reg);
void cpl(struct CPU *cpu);
void ccf(struct CPU *cpu);
void rr(struct CPU *cpu, uint8_t reg);
void rl(struct CPU *cpu, uint8_t reg);
void daa(struct CPU *cpu);


void load_reg(struct CPU *cpu, unsigned char reg, unsigned char amount);
void load_reg_16(struct CPU *cpu, unsigned char reg, unsigned char low, unsigned char high);
void set_mem(struct CPU *cpu, uint16_t addr, uint8_t amount);
void add(struct CPU *cpu, uint8_t reg, unsigned char amount);
void add_16(struct CPU *cpu, uint8_t reg, uint16_t amount);
void adc(struct CPU *cpu, uint8_t amount);
void sub(struct CPU *cpu, unsigned char amount);
void decrement_8(struct CPU *cpu, uint8_t reg);
void decrement_16(struct CPU *cpu, uint8_t reg);
void compare(struct CPU *cpu, unsigned char amount);
void and(struct CPU *cpu, unsigned char amount);
void or_8(struct CPU *cpu, uint8_t amount);
void or(struct CPU *cpu, unsigned char reg);
void xor_n(struct CPU *cpu, uint8_t amount);
void xor(struct CPU *cpu, unsigned char reg);

void push(struct CPU *cpu, uint8_t reg);
void pop(struct CPU *cpu, uint8_t reg);
void call(struct CPU *cpu, uint8_t high, uint8_t low);
void ret_function(struct CPU *cpu);

uint16_t* fetch_tile(struct CPU *cpu, uint16_t addr);
uint16_t interleave_tile(uint8_t low, uint8_t high);

uint16_t interleave_tile_pixel(uint8_t low, uint8_t high, uint8_t index);
