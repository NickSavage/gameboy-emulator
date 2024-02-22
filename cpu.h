#include <stdint.h>
#include <stdlib.h>

#define LCD_WIDTH 160
#define LCD_HEIGHT 144

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

struct CPU {
    uint8_t memory[65535];
    int8_t rom[32768];
    unsigned char regs[8];

    uint8_t clock;
    uint16_t pc;
    uint16_t sp;
    uint16_t ime; // interrupts register
    uint8_t halt;

    uint8_t keys; // currently pressed keys
};

enum ppu_mode {
    H_BLANK,
    V_BLANK,
    OAM_SCAN,
    DRAWING,
};

struct PPU {
    enum ppu_mode mode;
    int cycles;
    uint8_t dots;
    uint16_t buffer;

    uint32_t fb[LCD_HEIGHT][LCD_WIDTH];
};
