#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "opcodes.h"


void test_add() {
    unsigned char amount = 0b00010000;
    struct CPU cpu;
    load_reg(&cpu, 7, 0);
    add(&cpu, amount);

    assert(cpu.regs[7] == amount);
}

void test_add_16() {
    struct CPU cpu;
    load_reg(&cpu, REG_H, 0x98);
    load_reg(&cpu, REG_L, 0x34);
    load_reg(&cpu, REG_D, 0x00);
    load_reg(&cpu, REG_E, 0x0C);
    add_16(&cpu, 2, (cpu.regs[REG_D] << 8) + cpu.regs[REG_E]);
    assert(cpu.regs[REG_H] == 0x98);
    assert(cpu.regs[REG_L] == 0x40);
}

void test_sub() {
    unsigned char amount = 0b00010000;
    struct CPU cpu;
    load_reg(&cpu, 7, 19);
    sub(&cpu, amount);

    assert(cpu.regs[7] == 3);
}
void test_and() {
    struct CPU cpu;
    load_reg(&cpu, REG_A, 0b00000000);
    load_reg(&cpu, REG_B, 0b00000010);
    and(&cpu, cpu.regs[REG_B]);
    assert(cpu.regs[REG_A] == 0x00);
    assert(get_z_flag(&cpu) == 0);
    assert(get_h_flag(&cpu) == 1);
}

void test_xor() {
    struct CPU cpu;
    load_reg(&cpu, REG_A, 0);
    load_reg(&cpu, REG_B, 10);
    xor(&cpu, REG_B);
    assert(cpu.regs[REG_A] == 10);
    assert(get_z_flag(&cpu) == 0);
    assert(get_c_flag(&cpu) == 0);
    
    load_reg(&cpu, REG_A, 10);
    xor(&cpu, REG_A);
    assert(cpu.regs[REG_A] == 0);
    assert(get_z_flag(&cpu) == 1);
    assert(get_c_flag(&cpu) == 0);
}

void test_set_z_flag() {
    
    struct CPU cpu;
    load_reg(&cpu, REG_A, 0);
    set_z_flag(&cpu);
    assert(get_z_flag(&cpu) == 0);
    load_reg(&cpu, REG_A, 1);
    set_z_flag(&cpu);
}

void test_set_n_flag() {
    struct CPU cpu;
    set_n_flag(&cpu, 1);
    assert(get_n_flag(&cpu) == 1);
    set_n_flag(&cpu, 0);
    assert(get_n_flag(&cpu) == 0);
}
void test_set_c_flag() {
    struct CPU cpu;
    set_c_flag(&cpu, 1);
    assert(get_c_flag(&cpu) == 1);
    set_c_flag(&cpu, 0);
    assert(get_c_flag(&cpu) == 0);
}
void test_set_h_flag() {
    struct CPU cpu;
    set_h_flag(&cpu, 1);
    assert(get_h_flag(&cpu) == 1);
    set_h_flag(&cpu, 0);
    assert(get_h_flag(&cpu) == 0);
}

void test_set_mem() {
    struct CPU cpu;
    uint16_t addr = 0xFF40;
    assert(cpu.memory[addr] == 0);
    set_mem(&cpu, addr, 1);
    assert(cpu.memory[addr] == 1);
}

void test_push_pop() {
    struct CPU cpu;
    load_reg(&cpu, REG_B, 0b11110000);
    load_reg(&cpu, REG_C, 0b00001111);
    cpu.sp=0x8000;
    push(&cpu, 0);
    assert(cpu.sp == 0x7ffe);
    assert(cpu.memory[0x7fff] == 0b11110000);
    assert(cpu.memory[0x7ffe] == 0b00001111);
    load_reg(&cpu, REG_B, 0);
    load_reg(&cpu, REG_C, 0);
    assert(cpu.regs[REG_B] == 0);
    assert(cpu.regs[REG_B] == 0);
    pop(&cpu, 0);
    assert(cpu.sp == 0x8000);
    assert(cpu.regs[REG_B] == 0b11110000);
    assert(cpu.regs[REG_C] == 0b00001111);
}

void test_call_ret() {
    struct CPU cpu;
    
    cpu.pc = 0x123;
    cpu.sp = 0xaabb;
    call(&cpu, 0x0a, 0x00);
    assert(cpu.pc == 0xa00);
    assert(cpu.sp == 0xaab9);
    assert(cpu.memory[cpu.sp] == 0x23);
    assert(cpu.memory[cpu.sp + 1] == 0x01);
    ret_function(&cpu);
    assert(cpu.pc == 0x123);
    assert(cpu.sp == 0xaabb);
}

int main() {
    test_add();
    test_add_16();
    test_sub();
    test_xor();
    test_set_n_flag();
    test_set_c_flag();
    test_set_h_flag();
    test_set_mem();
    test_push_pop();
    test_call_ret();
    printf("\n");
}
