#include <stdlib.h>

enum ppu_mode {
    H_BLANK,
    V_BLANK,
    OAM_SCAN,
    DRAWING,
};

struct PPU {
    enum ppu_mode mode;
    int cycles;
};

