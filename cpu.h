#include <stdlib.h>

struct CPU {
    int8_t memory[65535];
    int8_t rom[32768];
    int16_t pc;
    int8_t regs[8];
};
