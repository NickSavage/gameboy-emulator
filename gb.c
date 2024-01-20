#include <stdio.h>
#include <stdlib.h>

#include "opcodes.h"

static const char *regNames[] = {"b", "c", "d", "e", "h", "l", "f", "a"};

int16_t ppc;

void init_cpu(struct CPU *cpu) {
    cpu->pc = 36;
    cpu->memory[0xFF44] = 144; // plugged, relates to vblank
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

int check_flag_c(struct CPU *cpu) {
    int ret = (cpu->regs[REG_F] & 0b00010000) >> FLAG_C;
    return ret;
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
	cpu->regs[reg] = second;
	ret = 1;
    }
    else if ((first & 0b11111111) == 0b11111010 ) {
	// ld a, [nn]
	printByteAsBinary(second);
	putchar(' ');
	printByteAsBinary(third);
	putchar(' ');
	int addr = (third << 8) + second;
	printf(" ld a, [%x]", addr);
	load_reg(cpu, REG_A, cpu->memory[addr]);

	ret = 2;
    }
    else if ((first & 0b11111111) == 0b11101010) {
	// ld [nn], a
	printByteAsBinary(second);
	putchar(' ');
	printByteAsBinary(third);
	putchar(' ');
	int addr = (third << 8) + second;
	printf(" ld [%x], a", addr);
	set_mem(cpu, addr, cpu->regs[REG_A]);
	ret = 2;
    }
    else if ((first & 0b11000111) == 0b01000110) {
	// ld r, (HL)
	int addr = (cpu->regs[REG_H] << 8) + cpu->regs[REG_L];
	printf(" -- %d --", addr);
	printf(" ld a, [HL]");
	load_reg(cpu, REG_A, cpu->memory[addr]);
    }
    else if ((first & 0b11111000) == 0b01110000) {
	// ld (HL), r
	
	int addr = (cpu->regs[REG_H] << 8) + cpu->regs[REG_L];
	int src = (first & 0b00000111);
	
	printf(" ld [HL], %s", regNames[src]);

	cpu->memory[addr] = cpu->regs[src];
    }
    else if ((first & 0b11000000) == 0b01000000) {
	// ld r, r
	// load reg (reg)
	int dest = (first & 0b00111000) >> 3;
	int src = (first & 0b00000111);
	printf(" ld %s, %s", regNames[dest], regNames[src]);
	cpu->regs[dest] = cpu->regs[src];
    }
    else if ((first & 0b11111111) == 0b00110110) {
	// ld (HL), n

	int addr = (cpu->regs[REG_H] << 8) + cpu->regs[REG_L];
	
	cpu->memory[addr] = second;
	ret = 1;
    }
    else if ((first & 0b11111111) == 0b00001010) {
	// ld a, (BC)
	
	int addr = (cpu->regs[REG_B] << 8) + cpu->regs[REG_C];
	
	cpu->regs[REG_A] = cpu->memory[addr];
	ret = 1;
    }
    else if ((first & 0b11111000) == 0b10000000) {
	// add r
	int reg = (first & 0b00111000) >> 3;
	printf(" add %s", regNames[reg]);
	
	add(cpu, cpu->regs[reg]);
    }
    else if ((first & 0b11111111) == 0b11000110) {
	// add n
	printByteAsBinary(second);
	putchar(' ');
	printf(" add a, %d", second);
	add(cpu, second);

	ret = 1;
    }
    else if ((first & 0b11111000) == 0b10010000) {
	// sub r
	int reg = (first & 0b00111000) >> 3;
 	printf(" sub a, %s", regNames[reg]);
	sub(cpu, cpu->regs[reg]);
	
	ret = 1;
    }
    else if ((first & 0b11111111) == 0b11111110) {
	//cp n
	printByteAsBinary(second);
	putchar(' ');
	printf(" cp %d", second);

	compare(cpu, second);
	ret = 1;
    }
    else if ((first & 0b11111000) == 0b10101000) {
	int reg = (first & 0b00000111);
	printf( "xor %s", regNames[reg]);

	xor(cpu, reg);
    }
    else if ((first & 0b11111111) == 0b11010110) {
	// sub n
	printByteAsBinary(second);
	putchar(' ');
	printf(" sub a, %d", second);
	sub(cpu, second);
	ret = 1;
    }
    else if ((first & 0b11111111) == 0b00100010) {
	// ld (HL+), a
	int addr = (cpu->regs[REG_H] << 8) + cpu->regs[REG_L];
	printf(" -- %d --", addr);
	
	printf(" ld [HL+], a");
	cpu->memory[addr] = cpu->regs[REG_A];
	
	addr -= 1;
	cpu->regs[REG_H] = addr >> 8;
	cpu->regs[REG_L] = addr & 0xFF;
	
    }
    else if ((first & 0b11111111) == 0b11000011) {
	// jp
	int addr = (third << 8) + second;
	printf(" JP %d", addr);
	ppc = addr - 1;
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

	if (cc == COND_NC && !(check_flag_c(cpu) == 1)) {
	    ppc = addr - 1;
	} else if (cc == COND_C && (check_flag_c(cpu) == 1)) {
	    ppc = addr - 1;
	} else {
	    
	    ret = 2;
	}
	
    }
    else if ((first & 0b11100111) == 0b00100000) {
	// jr cc, nn
	printByteAsBinary(second);
	putchar(' ');
	printByteAsBinary(third);
	putchar(' ');
	
	int cc = (first & 0b00011000) >> 3;
	printf(" JR %d, %d", cc, (int8_t)second);

	ret = 1;
	if (cc == COND_C && (check_flag_c(cpu) == 1)) {
	    ppc += (int8_t)second;
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
    for (int i = 0; i < size; i++) {
	cpu.rom[i] = data[i];
    }
    ppc = 0x0100;
    
    printf("%d", cpu.rom);
    while(running != 0) {
	printf("%x - ", ppc);
	printByteAsBinary(cpu.rom[ppc]);
	putchar(' ');
	ppc += parse_opcode(&cpu, ppc);
	ppc++;
	if (ppc > size) {
	    running = 0;
	}

	if (ppc == 15) {
	    break;
	}
    }
    putchar('\n');
    for (int i = 0; i < 8; i+=2) {
	printf("%s%s:", regNames[i],regNames[i + 1]);
	printByteAsBinary(cpu.regs[i]);
	putchar(' ');
	printByteAsBinary(cpu.regs[i+1]);
	putchar('\n');
    }
    // Free the memory
    free(data);

    return 0;
}
