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
    load_reg(&cpu, REG_B, 10);
    xor(&cpu, REG_B);
    assert(cpu.regs[REG_B] == 10);
    
}

int main() {
    test_add();
    test_sub();
    test_xor();
    printf("\n");
}
