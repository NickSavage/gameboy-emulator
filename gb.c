#include <stdio.h>
#include <stdlib.h>

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

static const char *regNames[] = {"b", "c", "d", "e", "h", "l", "f", "a"};

int16_t ppc;

int8_t regs[] = {0,0,0,0,0,0,0,0};

struct CPU {
    int8_t memory[65535];
    int8_t rom[32768];
    int16_t pc;
    //int8_t regs[8];
};

void init_cpu(struct CPU *cpu) {
    cpu->pc = 36;
}
    

void printByteAsBinary(unsigned char byte) {
    for (int i = 7; i >= 0; i--) {
        putchar((byte & (1 << i)) ? '1' : '0');
    }
}

unsigned char* readFile(char *filename, size_t* size) {
    
    FILE *file;
    unsigned char *buffer;
    long file_size;

    // Open the file
    file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }
    // Get the file size
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);
    printf("file size: %d\n", file_size);

    // Allocate memory for the entire file
    buffer = (unsigned char *)malloc(sizeof(unsigned char) * file_size);
    if (buffer == NULL) {
        fputs("Memory error", stderr);
        fclose(file);
        return NULL;
    }

    // Copy the file into the buffer
    if (fread(buffer, 1, file_size, file) != file_size) {
        fputs("Reading error", stderr);
        fclose(file);
        free(buffer);
        return NULL;
    }

    *size = file_size;
    // Close the file
    fclose(file);
    return buffer;
};

int check_flag_c() {
    int ret = (regs[REG_F] & 0b00010000) >> FLAG_C;
    return ret;
}
void set_z_flag() {
    if (regs[REG_A] == 0) {
	regs[REG_F] |= 1 << FLAG_Z;
    } else {
	regs[REG_F] |= 0 << FLAG_Z;
    }
}

void add(unsigned char amount) {
    regs[REG_A] += amount;

    set_z_flag();
    
    regs[REG_F] |= 0 << FLAG_N;
    printf("%d, %d", regs[REG_A], amount);
    if ((regs[REG_A]) < 0) {
	regs[REG_F] |= 1 << FLAG_C;
    } else {
	regs[REG_F] |= 0 << FLAG_C;
    }
}

void sub(unsigned char amount) {
    regs[REG_A] -= amount;
    
    set_z_flag();
    regs[REG_F] |= 1 << FLAG_N;
    if ((regs[REG_A] - amount) < 0) {
	regs[REG_F] |= 1 << FLAG_C;
    } else {
	regs[REG_F] |= 0 << FLAG_C;
    }
    
}

int parse_opcode(struct CPU *cpu, int pc) {
    
    unsigned char first = cpu->rom[pc];
    unsigned char second = cpu->rom[pc + 1];
    unsigned char third = cpu->rom[pc + 2];

    int ret = 0;
    if ((first & 0b11000111) == 0b00000110) {
	// ld r, n
	// load register immediate
	printByteAsBinary(second);
	int reg = (first & 0b00111000) >> 3;
	printf(" ld %s, %d", regNames[reg], second);
	printf(" -- %d -- ", reg);
	regs[reg] = second;
	ret = 1;
    }
    else if ((first & 0b11000111) == 0b01000110) {
	// ld r, (HL)
	int addr = (regs[REG_H] << 8) + regs[REG_L];
	printf(" -- %d --", addr);
	printf(" ld a, [HL]");
	regs[REG_A] = cpu->memory[addr];
    }
    else if ((first & 0b11111000) == 0b01110000) {
	// ld (HL), r
	
	int addr = (regs[REG_H] << 8) + regs[REG_L];
	int src = (first & 0b00000111);
	
	printf(" ld [HL], %s", regNames[src]);

	cpu->memory[addr] = regs[src];
    }
    else if ((first & 0b11000000) == 0b01000000) {
	// ld r, r
	// load reg (reg)
	int dest = (first & 0b00111000) >> 3;
	int src = (first & 0b00000111);
	printf(" ld %s, %s", regNames[dest], regNames[src]);
	regs[dest] = regs[src];
    }
    else if ((first & 0b11111111) == 0b00110110) {
	// ld (HL), n

	int addr = (regs[REG_H] << 8) + regs[REG_L];
	
	cpu->memory[addr] = second;
	ret = 1;
    }
    else if ((first & 0b11111111) == 0b00001010) {
	// ld a, (BC)
	
	int addr = (regs[REG_B] << 8) + regs[REG_C];
	
	regs[REG_A] = cpu->memory[addr];
	ret = 1;
    }
    else if ((first & 0b11111000) == 0b10000000) {
	// add r
	int reg = (first & 0b00111000) >> 3;
	printf(" add %s", regNames[reg]);
	
	add(regs[reg]);
    }
    else if ((first & 0b11111111) == 0b11000110) {
	// add n
	printByteAsBinary(second);
	putchar(' ');
	printf(" add a, %d", second);
	add(second);

	ret = 1;
    }
    else if ((first & 0b11111000) == 0b10010000) {
	// sub r
	int reg = (first & 0b00111000) >> 3;
 	printf(" sub a, %s", regNames[reg]);
	sub(regs[reg]);
	
	ret = 1;
    }
    else if ((first & 0b11111111) == 0b11010110) {
	// sub n
	printByteAsBinary(second);
	putchar(' ');
	printf(" sub a, %d", second);
	sub(second);
	ret = 1;
    }
    else if ((first & 0b11111111) == 0b00100010) {
	// ld (HL+), a
	int addr = (regs[REG_H] << 8) + regs[REG_L];
	printf(" -- %d --", addr);
	
	printf(" ld [HL+], a");
	cpu->memory[addr] = regs[REG_A];
	
	addr -= 1;
	regs[REG_H] = addr >> 8;
	regs[REG_L] = addr & 0xFF;
	
    }
    else if ((first & 0b11111111) == 0b11000011) {
	// jp
	int addr = (third << 8) + second;
	printf(" JP %d", addr);
	ppc = addr;
    }
    else if ((first & 0b11100111) == 0b11000010) {
	// jp cc, nn
	printByteAsBinary(second);
	putchar(' ');
	printByteAsBinary(third);
	putchar(' ');

	int cc = (first & 0b00011000) >> 3;
	int addr = (third << 8) + second;
	printf(" JP %d, %d", cc, addr);

	if (cc == COND_NC && !(check_flag_c() == 1)) {
	    ppc = addr;
	} else if (cc == COND_C && (check_flag_c() == 1)) {
	    ppc = addr;
	}
	
    }
    putchar('\n');
    return ret;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    size_t size;

    int running = 1;
    
    struct CPU cpu;
    init_cpu(&cpu);
    printf("%d", cpu.pc);

    unsigned char *data = readFile(argv[1], &size);
    for (int i; i < size; i++) {
	cpu.rom[i] = data[i];
    }
    ppc = 36;
    
    printf("%d", cpu.rom);
    while(running != 0) {
	printByteAsBinary(cpu.rom[ppc]);
	putchar(' ');
	ppc += parse_opcode(&cpu, ppc);
	ppc++;
	if (ppc > size) {
	    running = 0;
	}
    }
    putchar('\n');
    for (int i = 0; i < 8; i+=2) {
	printf("%s%s:", regNames[i],regNames[i + 1]);
	printByteAsBinary(regs[i]);
	putchar(' ');
	printByteAsBinary(regs[i+1]);
	putchar('\n');
    }
    // Free the memory
    free(data);

    return 0;
}
