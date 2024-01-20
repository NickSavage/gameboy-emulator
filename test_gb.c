#include <stdio.h>
#include <assert.h>

#include "opcodes.h"


void test_add() {
    unsigned char amount = 0b00010000;
    struct CPU cpu;
    load_reg(&cpu, 7, 0);
    add(&cpu, amount);

    assert(cpu.regs[7] == amount);
}

void test_sub() {
    unsigned char amount = 0b00010000;
    struct CPU cpu;
    load_reg(&cpu, 7, 19);
    sub(&cpu, amount);

    assert(cpu.regs[7] == 3);
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



int main() {
    test_add();
    test_sub();
    test_xor();
    test_set_n_flag();
    test_set_c_flag();
    test_set_h_flag();
    printf("\n");
}
