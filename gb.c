#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <unistd.h>

#include <SDL2/SDL.h>

#include "opcodes.h"

#define NUM_KEYS    8
uint32_t KEYS[] =
{
	SDLK_RIGHT, // control map one
	SDLK_LEFT,
	SDLK_UP,
	SDLK_DOWN,
	SDLK_z,
	SDLK_x,
	SDLK_RSHIFT,
	SDLK_RETURN,
	SDLK_d,     // control map two
	SDLK_a,
	SDLK_w,
	SDLK_s,
	SDLK_SPACE,
	SDLK_BACKSPACE,
	SDLK_LSHIFT,
	SDLK_ESCAPE
};

static const char *regNames[] = {"b", "c", "d", "e", "h", "l", "f", "a"};
static const char *reg_names_16[] = {"bc", "de", "hl", "sp"};

void quit(int sig) {
    if (sig == SIGINT) {
	exit(0);
    }
}

void output_registers(struct CPU *cpu) {
    putchar('\n');
    for (int i = 0; i < 8; i+=2) {
	printf("%s%s:", regNames[i],regNames[i + 1]);
	printByteAsBinary(cpu->regs[i]);
	putchar(' ');
	printByteAsBinary(cpu->regs[i+1]);
	putchar('\n');
    }
}

void init_screen(struct CPU *cpu) {

    cpu->memory[0xFF42] = 0; // store scy in 0xFF42
    cpu->memory[0xFF43] = 0; // store scy in 0xFF42
}

void init_cpu(struct CPU *cpu) {

    cpu->clock = 0;
    cpu->pc = 0x0100;
    cpu->sp = 0xFFFE;
    cpu->memory[0xFF44] = 144; // plugged, relates to vblank
    for (int i = 0x0000; i < 0x7fff; i++) {
	cpu->memory[i] = cpu->rom[i];
    }
}
    
void init_ppu(struct PPU *ppu) {
    ppu->dots = 0;
}

unsigned char* readFile(char *filename, int* size) {
    
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

void output_memory(struct CPU *cpu) {
    FILE *file = fopen("output.bin", "wb");

    // Write the array to the file
    size_t elements_written = fwrite(cpu->memory, sizeof(uint8_t), 65535, file);
    if (elements_written != 65535) {
        perror("Failed to write the full array");
        // Close the file before returning
        fclose(file);
    }
}

int check_flag_c(struct CPU *cpu) {
    int ret = (cpu->regs[REG_F] & 0b00010000) >> FLAG_C;
    return ret;
}

int parse_cb_opcode(struct CPU *cpu, int pc) {
    unsigned char first = cpu->memory[cpu->pc];

    uint8_t reg;
    int result = 0;
    switch(first) {
    case (0x30): case (0x31): case(0x32): case(0x33): case(0x34): case(0x35): case(0x36): case(0x37):
	reg = first & 0b00000111;
	printf(" swap %s", regNames[reg]);
	swap(cpu, reg);
	break;
    case (0x38): case (0x39): case(0x3a): case(0x3b): case(0x3c): case(0x3d): case(0x3e): case(0x3f):
	reg = first & 0b00000111;

	printf(" srl %s", regNames[reg]);
	srl(cpu, reg);

	break;
    case (0x40): case (0x41): case (0x42): case (0x43): case (0x44): case (0x45): case (0x46): case (0x47): 
	// bit 0, r
	reg = first & 0b00000111;
	bit(cpu, 0, reg);
	cpu->clock += 1;
	break;
    case (0x48): case (0x49): case (0x4a): case (0x4b): case (0x4c): case (0x4d): case (0x4e): case (0x4f): 
	// bit 0, r
	reg = first & 0b00000111;
	bit(cpu, 1, reg);
	cpu->clock += 1;
	break;
    case (0x50): case (0x51): case (0x52): case (0x53): case (0x54): case (0x55): case (0x56): case (0x57): 
	// bit 0, r
	reg = first & 0b00000111;
	bit(cpu, 2, reg);
	cpu->clock += 1;
	break;
    case (0x58): case (0x59): case (0x5a): case (0x5b): case (0x5c): case (0x5d): case (0x5e): case (0x5f): 
	// bit 0, r
	reg = first & 0b00000111;
	bit(cpu, 3, reg);
	cpu->clock += 1;
	break;
    case (0x60): case (0x61): case (0x62): case (0x63): case (0x64): case (0x65): case (0x66): case (0x67): 
	// bit 0, r
	reg = first & 0b00000111;
	bit(cpu, 4, reg);
	cpu->clock += 1;
	break;
    case (0x68): case (0x69): case (0x6a): case (0x6b): case (0x6c): case (0x6d): case (0x6e): case (0x6f): 
	// bit 0, r
	reg = first & 0b00000111;
	bit(cpu, 5, reg);
	cpu->clock += 1;
	break;
    case (0x70): case (0x71): case (0x72): case (0x73): case (0x74): case (0x75): case (0x76): case (0x77): 
	// bit 0, r
	reg = first & 0b00000111;
	bit(cpu, 6, reg);
	cpu->clock += 1;
	break;
    case (0x78): case (0x79): case (0x7a): case (0x7b): case (0x7c): case (0x7d): case (0x7e): case (0x7f): 
	// bit 0, r
	reg = first & 0b00000111;
	bit(cpu, 7, reg);
	cpu->clock += 1;
	break;
    case (0x87):
	printf(" res 0, a");
	reg = REG_A;
	reset_bit(cpu, reg, 0);
	break;
    default:
	printf("unimplemented cb opcode");
	exit(0);

    }
    putchar('\n');
    return result;
}

int parse_opcode(struct CPU *cpu) {
    
    unsigned char first = cpu->memory[cpu->pc];
    unsigned char second = cpu->memory[cpu->pc + 1];
    unsigned char third = cpu->memory[cpu->pc + 2];
    uint16_t addr;
    uint8_t reg;
    uint8_t n;

    int found = 1;
    int ret = 0;
    uint8_t cc;
    
    switch (first) {
    case(0x00):
	printf(" noop");
	break;
    case(0x04): case(0x14): case(0x24): case(0x0c): case(0x1c):case(0x2c): case(0x3c):
	// inc r
	reg = (first & 0b00111000) >> 3;
	printf(" inc %s", regNames[reg]);
	add(cpu, reg, 1);

	break;
    case (0x34):
	// inc [hl]
	printf(" inc [hl]");
	addr = (cpu->regs[REG_H] << 8) + cpu->regs[REG_L];
	cpu->memory[addr] +=1;

	cpu->clock += 2;
	break;
	
	
    case(0x06): case(0x16): case(0x26): case(0x0e): case(0x1e): case(0x2e): case(0x3e): case(0x4e):
	// ld r, n
	printByteAsBinary(second);
	int reg = (first & 0b00111000) >> 3;
	printf(" ld %s, $%x", regNames[reg], second);
	load_reg(cpu, reg, second);
	ret = 1;

	cpu->clock += 1;
	break;
    case (0x08):
	// ld (nn), SP
	printf(" ld [%x], sp", (second << 8) + third);
	cpu->memory[second] = cpu->sp >> 8; // low
	cpu->memory[third] = cpu->sp & 0x0F; // high
	ret = 2;
	cpu->clock += 4;
	break;

    case (0x90):case (0x91):case (0x92):case (0x93):case (0x94):case (0x95):case (0x96):case (0x97):
	reg = (first & 0b00000111);
	printf(" sub a, %s", regNames[reg]);
	sub(cpu, cpu->regs[reg]);
	break;
	
    case (0xc0): case(0xd0): case (0xc8): case(0xd8):
	// ret cc
	cc = (first & 0b00011000) >> 3;
	printf(" ret %d", cc);
	if (cc == COND_Z && get_z_flag(cpu) == 1) {
	    ret_function(cpu);
	    cpu->clock += 4;
	} else {
	    cpu->clock += 1;
	}
	break;
    case(0xc9):
	// ret

	printf(" ret");
	ret_function(cpu);
	cpu->clock += 3;
	//	cpu->pc -= 1;

	break;
	    
    case (0x2a):
	// ld a, (HL+)
	printf("ld a, [hl+]");
	addr = (cpu->regs[REG_H] << 8) + cpu->regs[REG_L];
	load_reg(cpu, REG_A, cpu->memory[addr]);

	int total = (cpu->regs[REG_H] << 8) + cpu->regs[REG_L] + 1;
	int high = (total >> 8) & 0xFF;
	int low = total & 0xFF;

	cpu->regs[REG_H] = (total >> 8) & 0xFF;
	cpu->regs[REG_L] = total & 0xFF;

	cpu->clock += 1;
	
	break;
    case (0x3f):
	printf(" ccf");
	ccf(cpu);
	break;
    case (0x88): case (0x89): case (0x8a): case (0x8b): case (0x8c): case (0x8d): case (0x8e): case (0x8f):
	// adc r
	reg = first & 0b00000111;
	printf("adc %s", regNames[reg]);
	adc(cpu, cpu->regs[reg]);
	break;
    case (0xce):
        printf(" adc a, %d", second);
	adc(cpu, second);
	cpu->clock += 1;
	ret = 1;
	break;
    case(0xcb):
	// cb, prefix, still not really sure what this is
	printf("cb");
	cpu->pc += 1;
	ret = parse_cb_opcode(cpu, cpu->pc + 1);

	return ret;
	
    case(0xcd):
	// call nn
	addr = (third << 8) + second;
	printf(" call [%x]", addr);
	cpu->pc += 2;
	call(cpu, third, second);

	printf("%x, %x, %x \n", cpu->sp, cpu->memory[cpu->sp + 1], cpu->memory[cpu-> sp]);
	//	output_memory(cpu);
	
	cpu->pc = addr - 1;
	cpu->clock += 5;
	break;
    case (0xd9):
	// reti
	printf("reti");
	ret_function(cpu);
	cpu->ime = 1;
	cpu->pc -= 1; // with vblank interrupts, it sets the counter one higher than it should be
	break;
    case(0xe2):
	addr = 0xFF00 + cpu->regs[REG_C];
	printf(" ldh [c], a - [$%x]", addr);
	cpu->memory[addr] = cpu->regs[REG_A];
	cpu->clock += 1;
	break;
    case(0xe6):
	printf("and $%x", second);
	and(cpu, second);
	ret = 1;
	cpu->clock += 1;
	break;
    case(0xf6):
	// or a, n
	printf("or a, $%x", second);
	or_8(cpu, second);
	ret = 1;
	cpu->clock += 1;
	break;
	
    case(0xfb):
	printf(" ei");
	cpu->ime = 1;
	cpu->clock += 2;
	break;
    case (0xff):
	// rst 
	addr = (first & 00111000) >> 3;
	printf( "rst %x", addr);
	call(cpu, 0, addr);
	break;
    default:
	found = 0;
    }

    
    if (found == 1) {
	
	putchar('\n');
	return ret;
    }
    if (first == 0b00110110) {
	// ld [HL], n
	printByteAsBinary(second);
	printf("ld [HL], %d", second);
	uint16_t addr = (cpu->regs[REG_H] << 8) + cpu->regs[REG_L];
	cpu->memory[addr] = second;

	cpu->clock += 2;
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

	cpu->clock += 3;
	ret = 2;
    }
    else if ((first & 0b11111111) == 0b11101010) {
	// ld [nn], a
	printByteAsBinary(second);
	putchar(' ');
	printByteAsBinary(third);
	putchar(' ');
	uint16_t addr = (third << 8) + second;
	printf("%d", addr);
	printf(" ld [%x], a", addr);
	set_mem(cpu, addr, cpu->regs[REG_A]);
	cpu->clock += 3;
	ret = 2;
    }
    else if ((first & 0b11000111) == 0b01000110) {
	// ld r, (HL)
	int addr = (cpu->regs[REG_H] << 8) + cpu->regs[REG_L];
	printf(" -- %d --", addr);
	printf(" ld a, [HL]");
	load_reg(cpu, REG_A, cpu->memory[addr]);
	cpu->clock += 1;
    }
    else if ((first & 0b11111000) == 0b01110000) {
	// ld (HL), r
	
	int addr = (cpu->regs[REG_H] << 8) + cpu->regs[REG_L];
	int src = (first & 0b00000111);
	
	printf(" ld [HL], %s", regNames[src]);

	cpu->memory[addr] = cpu->regs[src];
	cpu->clock += 1;
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
	cpu->clock += 3;
    }
    else if ((first & 0b11001111) == 0b00000001) {
	// LD rr, nn

	printByteAsBinary(second);
	putchar(' ');
	printByteAsBinary(third);
	putchar(' ');
	
	int low = second;
	int high = third;
	int reg = (first & 0b00110000) >> 4;

	printf("ld %s, %x", reg_names_16[reg], (high << 8) + low);
	load_reg_16(cpu, reg, low, high);
	ret = 2;
	cpu->clock += 2;
    }
    else if ((first & 0b11111111) == 0b00001010) {
	// ld a, (BC)
	
	int addr = (cpu->regs[REG_B] << 8) + cpu->regs[REG_C];
	
	printf(" ld a, [BC]");
	cpu->regs[REG_A] = cpu->memory[addr];
	ret = 1;
	cpu->clock += 1;
    }
    else if ((first & 0b11111111) == 0b00011010) {
	// ld a, (DE)
	
	int addr = (cpu->regs[REG_D] << 8) + cpu->regs[REG_E];
	
	printf(" ld a, [DE]");
	cpu->regs[REG_A] = cpu->memory[addr];
	cpu->clock += 1;
    }
    else if (first == 0b00010010) {
	// ld [DE], a
	int addr = (cpu->regs[REG_D] << 8) + cpu->regs[REG_E];
	
	printf(" ld [DE], a");
	cpu->memory[addr] = cpu->regs[REG_A];

	cpu->clock += 1;
    }
    else if (first == 0b11110000) {
	// ldh a, 0xFF00 + n
	printf(" ldh a, [%x]", 0xFF00 + second);
	cpu->regs[REG_A] = cpu->memory[0xFF00+second];
	ret = 1;

	cpu->clock += 2;
    }
    else if (first == 0b11100000) {
	// ldh 
	printf(" ldh [%x], a", 0xFF00 + second);
	ret = 1;

	if (0xFF00 + second == 0xff00) {
	    // read only first nibble for 0xFF00
	    printf("asdsad");
	    cpu->memory[0xFF00] = (cpu->regs[REG_A] & 0b11110000) + (cpu->memory[0xFF00] & 0b00001111);
	} else {
	    
	    cpu->memory[0xFF00+second] = cpu->regs[REG_A];
	}
	if (0xFF00 + second == 0xFF46) {
	    // oam dma transfer

	    for (int i = 0; i < 160; i++) {
		cpu->memory[0xFE00 + i] = cpu->memory[(cpu->regs[REG_A] << 8) + i];
	    }
	}
	
	cpu->clock += 2;
    }
    else if ((first & 0b11111000) == 0b10000000) {
	// add r
	int reg = (first & 0b00000111);
	printf(" add %s", regNames[reg]);
	
	add(cpu, REG_A, cpu->regs[reg]);
    }
    else if ((first & 0b11111111) == 0b11000110) {
	// add n
	printByteAsBinary(second);
	putchar(' ');
	printf(" add a, %d", second);
	add(cpu, REG_A, second);

	ret = 1;
	cpu->clock += 1;
    }
    else if (first == 0x09) {
	// add hl, de
	printf(" add hl, bc");
	add_16(cpu, 2, (cpu->regs[REG_B] << 8) + cpu->regs[REG_C]);
	cpu->clock += 1;
    }
    else if (first == 0x19) {
	// add hl, de
	printf(" add hl, de");
	add_16(cpu, 2, (cpu->regs[REG_D] << 8) + cpu->regs[REG_E]);
	cpu->clock += 1;
    }
    else if (first == 0x29) {
	// add hl, de
	printf(" add hl, hl");
	add_16(cpu, 2, (cpu->regs[REG_H] << 8) + cpu->regs[REG_L]);
	cpu->clock += 1;
    }
    /* else if ((first & 0b11111000) == 0b10010000) { */
    /* 	// sub r */
    /* 	int reg = (first & 0b00111000) >> 3; */
    /* 	printf(" sub a, %s", regNames[reg]); */
    /* 	sub(cpu, cpu->regs[reg]); */
	
    /* 	ret = 1; */
    /* } */
    else if ((first & 0b11111111) == 0b11111110) {
	//cp n
	printByteAsBinary(second);
	putchar(' ');
	printf(" cp %x", second);

	printf("- %x %x", cpu->regs[REG_A], second);
	compare(cpu, second);
	//	output_registers(cpu);
	ret = 1;
    }
    else if ((first & 0b11111000) == 0b10100000) {
	// and r
	int reg = (first & 0b00000111);
	printf( "and %s", regNames[reg]);
	and(cpu, cpu->regs[reg]);
    }
    else if ((first & 0b11111000) == 0b10110000) {
	int reg = (first & 0b00000111);
	printf( "or %s", regNames[reg]);
	or(cpu, reg);

    }
    else if ((first & 0b11111000) == 0b10101000) {
	// xor r
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

	cpu->clock += 1;
    }
    else if ((first & 0b11111111) == 0b00100010) {
	// ld (HL+), a
	int addr = (cpu->regs[REG_H] << 8) + cpu->regs[REG_L];
	
	printf(" ld [HL+], a");
	set_mem(cpu, addr, cpu->regs[REG_A]);
	
	addr += 1;
	cpu->regs[REG_H] = addr >> 8;
	cpu->regs[REG_L] = addr & 0xFF;
	
	cpu->clock += 1;
    }
    else if (first == 0x03) {
	printf("inc bc");
	int total = (cpu->regs[REG_B] << 8) + cpu->regs[REG_C] + 1;
	int high = (total >> 8) & 0xFF;
	int low = total & 0xFF;

	cpu->regs[REG_B] = high;
	cpu->regs[REG_C] = low;

	printf("; %x", (cpu->regs[REG_B] << 8) + cpu->regs[REG_C]);

	cpu->clock += 1;
    }
    else if (first == 0x13) {
	printf("inc de");
	int total = (cpu->regs[REG_D] << 8) + cpu->regs[REG_E] + 1;
	int high = (total >> 8) & 0xFF;
	int low = total & 0xFF;

	cpu->regs[REG_D] = high;
	cpu->regs[REG_E] = low;

	printf("; %x", (cpu->regs[REG_D] << 8) + cpu->regs[REG_E]);

	cpu->clock += 1;
    }
    else if (first == 0b00100011) {
	uint8_t reg = (first >> 4) & 0b0011;
	printf("inc hl");

	int total = (cpu->regs[REG_H] << 8) + cpu->regs[REG_L] + 1;
	int high = (total >> 8) & 0xFF;
	int low = total & 0xFF;

	cpu->regs[REG_H] = high;
	cpu->regs[REG_L] = low;

	cpu->clock += 1;
    }
    else if ((first & 0b11000111) == 0b00000101) {
	// dec r
	uint8_t reg = (first >> 3) & 0b00111;

	printf("dec %s", regNames[reg]);
	decrement_8(cpu, reg);
    }
    else if (first == 0x0B) {
	printf("dec bc");
	int total = (cpu->regs[REG_B] << 8) + cpu->regs[REG_C] - 1;
	int high = (total >> 8) & 0xFF;
	int low = total & 0xFF;

	cpu->regs[REG_B] = high;
	cpu->regs[REG_C] = low;
	printf("; %x", (cpu->regs[REG_B] << 8) + cpu->regs[REG_C]);

	cpu->clock += 1;
    }
    else if ((first & 0b11111111) == 0b11000011) {
	// jp
	int addr = (third << 8) + second;
	printf(" JP %x", addr);
	cpu->pc = addr - 1;

	cpu->clock += 3;
    }
    else if (first == 0xe9) {
	// jp hl
	int addr = (cpu->regs[REG_H] << 8) + cpu->regs[REG_L];
	printf(" JP hl - %x", addr);
	cpu->pc = addr - 1;
    }
    else if ((first & 0b11100111) == 0b11000010) {
	// jp cc, n
	printByteAsBinary(second);
	putchar(' ');
	printByteAsBinary(third);
	putchar(' ');

	int cc = (first & 0b00011000) >> 3;
	int addr = (third << 8) + second;
	printf(" JP %d, %d", cc, addr);

	if (cc == COND_NC && !(check_flag_c(cpu) == 1)) {
	    cpu->pc = addr - 1;

	    cpu->clock += 3;
	} else if (cc == COND_C && (check_flag_c(cpu) == 1)) {
	    cpu->pc = addr - 1;
	    cpu->clock += 3;
	} else {
	    
	    cpu->clock += 2;
	    ret = 2;
	}
	
    }
    else if ((first & 0b11111111) == 0b00011000) {
	// jr e
	printf(" jr %x", (int8_t)second);
	if ((int8_t)second < 1) {
	    cpu->pc += (int8_t)second;
	} else {
	    cpu->pc += (int8_t)second;
	}
	ret = 1;

	cpu->clock += 2;
    }
    else if ((first & 0b11100111) == 0b00100000) {
	// jr cc, nn
	printByteAsBinary(second);
	putchar(' ');
	
	int cc = (first & 0b00011000) >> 3;
	printf(" JR %d, %d", cc, (int8_t)second);

	ret = 1;
	if (cc == COND_C && (check_flag_c(cpu) == 1)) {
	    cpu->pc += (int8_t)second;

	    cpu->clock += 2;
	} else if (cc == COND_Z && (get_z_flag(cpu) == 1)) {
	    cpu->pc += (int8_t)second;

	    cpu->clock += 2;
	} else if (cc == COND_NZ && (get_z_flag(cpu) == 0)) {
	    printf("does htis get triggered?");
	    cpu->pc += (int8_t)second;
	    cpu->clock += 2;
	} else if (cc == COND_NC && (get_c_flag(cpu) == 0)) {
	    cpu->pc += (int8_t)second;
	    cpu->clock += 2;
	} else {
	    ret = 1;
	    cpu->clock += 1;
	}
	
	putchar('\n');
	for (int i = 0; i < 8; i+=2) {
	    printf("%s%s:", regNames[i],regNames[i + 1]);
	    printByteAsBinary(cpu->regs[i]);
	    putchar(' ');
	    printByteAsBinary(cpu->regs[i+1]);
	    putchar('\n');
	}
	putchar('\n');
	//sleep(1);
    }
    else if ((first & 0b11001111) == 0b11000101) {
	// push rr
	int reg = (first & 0b00110000) >> 4;
	printf(" push %s", reg_names_16[reg]);
	push(cpu, reg);

	cpu->clock += 3;
    }
    else if (first == 0xC1) {
	// pop rr
	printf(" pop bc");
	pop(cpu, 0);
	cpu->clock += 2;
    }
    else if (first == 0xD1) {
	// pop rr
	printf(" pop de");
	pop(cpu, 1);
	cpu->clock += 2;
    }
    else if (first == 0xE1) {
	// pop rr
	printf(" pop hl");
	pop(cpu, 2);
	cpu->clock += 2;
    }
    else if (first == 0xF1) {
	// pop rr
	printf(" pop af");
	pop(cpu, 3);
	cpu->clock += 2;
    }
    else if ((first & 0b11111111) == 0b11110011) {
	// disable interrupts
	cpu->ime = 0;
	printf(" di");
    }
    else {
	printByteAsBinary(second);
	putchar(' ');
	printByteAsBinary(third);
	printf(" unknown");
	ret = -1;
    }
    putchar('\n');
    return ret;
}

uint8_t bg_tile_map_mode(struct CPU *cpu) {
    uint8_t byte = cpu->memory[0xFF40];
    uint8_t result = ((byte & 0b00001000) >> 3) & 0b00001;
    return result;
}

uint8_t bg_tile_data_mode(struct CPU *cpu) {
    uint8_t byte = cpu->memory[0xFF40];
    uint8_t result = ((byte & 0b00010000) >> 4);
    return result;
    
}
uint32_t colourize_pixel(int input) {
    // The input is expected to be a value between 0 and 3
    // where 0 is white and 3 is black in the Game Boy's 2-bit color space.
    switch (input) {
        case 0: // White
            return 0xFFFFFFFF; // ARGB for white
        case 1: // Light gray
            return 0xFFAAAAAA; // ARGB for light gray
        case 2: // Dark gray
            return 0xFF555555; // ARGB for dark gray
        case 3: // Black
            return 0xFF000000; // ARGB for black
        default:
            // If the input is out of range, return a noticeable color (e.g., red)
            // to indicate an error or unexpected value.
            return 0xFFFF0000;
    }
}


void build_fb(struct CPU *cpu, struct PPU *ppu, uint8_t ly) {
    uint8_t bg_tile_map_mode_addr = bg_tile_map_mode(cpu);
    uint8_t bg_tile_data_mode_addr = bg_tile_data_mode(cpu) == 1 ? 0x8000 : 0x9000;
    uint16_t tile_index_addr = bg_tile_map_mode_addr == 0 ? 0x9800 : 0x9C00;
    uint16_t addr;
    uint8_t pixel;
    uint8_t colour_pixel;
    
    uint8_t tile_x;
    uint8_t tile_y;
    uint8_t tile_pixel_x;
    uint8_t tile_pixel_y;
    uint16_t tile_index;
    uint8_t tile_id;
/*     for (int x = 0; x < LCD_WIDTH; x++) { */
/* 	fb[x][0] = 0xFFFF; */
/* 	fb[x][1] = 0xFFFF; */
/*     } */
/* } */
    for (uint8_t x = 0; x <= 160; x++) {
	
	tile_x = x / 8;
	tile_y = ly / 8;
	tile_pixel_x = x % 8;
	tile_pixel_y = ly % 8;
	
	tile_index = tile_y * 32 + tile_x;
	tile_id = cpu->memory[tile_index_addr + tile_index]; 
	if (bg_tile_data_mode(cpu) == 1) {
	    addr = 0x8000 + tile_id * 16 + tile_pixel_y * 2;
	} 
	else {
	    addr = (tile_id > 127 ? 0x8800 : 0x9000) + tile_id * 16 + tile_pixel_y * 2;
	}
	pixel = interleave_tile_pixel(cpu->memory[addr], cpu->memory[addr + 1], 7 - tile_pixel_x);
	colour_pixel = colourize_pixel(pixel);
	
	ppu->fb[ly][x] = colour_pixel;
	//   printf("ly: %d, x: %d, tile_x: %d, tile_y: %d, tile_index: %d, pixel: %d\n", ly, x, tile_x, tile_y, tile_index, pixel);
    }
}

void render_sprites(struct CPU *cpu, struct PPU *ppu, uint8_t ly) {
    uint8_t tile_y_pos;
    uint8_t tile_x_pos;
    int8_t tile_y_diff;
    uint8_t tile_number;
    uint8_t sprite_flags;
    
    uint8_t low;
    uint8_t high;
    uint8_t i;
    
    uint16_t addr;
    uint8_t pixel;
    uint8_t colour_pixel;

    for (uint8_t sprite_number = 0; sprite_number < 40; sprite_number++) {
	tile_y_pos = cpu->memory[0xFE00 + sprite_number * 4];
	tile_x_pos = cpu->memory[0xFE00 + sprite_number * 4 + 1];
	tile_y_diff = tile_y_pos - ly - 16;
	tile_number = cpu->memory[0xFE00 + sprite_number * 4 + 2];
	sprite_flags = cpu->memory[0xFE00 + sprite_number * 4 + 3];

	if (tile_y_pos < 16 || tile_y_pos > 160) {
	    // not on screen
	    continue;
	}
	if (tile_x_pos < 8 || tile_x_pos > 160) {
	    // not on screen
	    continue;
	}
	if (ly + 8 < tile_y_pos && ly + 16 >= tile_y_pos) {
	    /* printf("sprite %d asd- ", sprite_number); */
	    addr = 0x8000 + tile_number * 16 + tile_y_diff * -2;
	    /* printf("%d, %d, %d, %d, %x, %x\n", ly, tile_y_pos, tile_x_pos, tile_y_diff, tile_number, addr); */

	    for (int x = 0; x < 8; x++) {
		i = (sprite_flags & 0b00100000 << 5) == 1 ? x : 7 - x; 
		pixel = interleave_tile_pixel(cpu->memory[addr], cpu->memory[addr + 1], i);
		if (!(pixel == 0)) {
		    colour_pixel = colourize_pixel(pixel);
		    ppu->fb[ly][tile_x_pos - 8 + x] = colour_pixel;
		}
	    }
	    
	}
    }
    for (uint8_t x = 0; x <= 160; x++) {
	
    }
    
}

void render_frame(struct CPU *cpu, struct PPU *ppu, SDL_Texture *tex, SDL_Renderer *ren) {
    uint8_t ly = cpu->memory[0xFF44];
    
    if (ly == 144) {
	request_vblank_int(cpu);
    } else if (ly > 144) {
	
    } else if ((cpu->memory[0xFF40] & 0b10000000) >> 7 == 1) {
	SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
	SDL_RenderClear(ren);
	
	build_fb(cpu, ppu, ly);
	render_sprites(cpu, ppu, ly);
	SDL_UpdateTexture(tex, NULL, ppu->fb, LCD_WIDTH * 4);
	SDL_RenderClear(ren);
	SDL_RenderCopy(ren, tex, NULL, NULL);
	SDL_RenderPresent(ren);
	//	SDL_Delay(20);
    }
    ly += 1;
    if (ly == 154) {
	ly = 0;
    }
    cpu->memory[0xFF44] = ly; // store ly in 0xFF44
}

void handle_interrupts(struct CPU *cpu) {
    if (cpu->ime == 1) {
	if ((cpu->memory[0xffff] & 0x01) == 1) {
	    // vblank
	    printf("vblank int");
	    call(cpu, 0x00, 0x40);
	}
    }
    cpu->memory[0xffff] = 0;
}
void UpdateP1(struct CPU *cpu)
{
    cpu->memory[0xFF00] |= 0x0F;
    if (!((cpu->memory[0xFF00] & 0x10) == 1))
	cpu->memory[0xFF00] &= 0xF0 | ((cpu->keys & 0x0F) ^ 0x0F);
    if (!((cpu->memory[0xFF00] & 0x20) == 1))
	cpu->memory[0xFF00] &= 0xF0 | (((cpu->keys >> 4) & 0x0F) ^ 0x0F);
    if (cpu->keys == 0) {
	cpu->memory[0xFF00] = cpu->memory[0xFF00] & 0b11110000;
    }
}

void KeyPress(struct CPU *cpu, uint8_t key)
{
    cpu->keys |= 0x01 << key;
    UpdateP1(cpu);
}

void KeyRelease(struct CPU *cpu, uint8_t key)
{
    cpu->keys = 0;
    UpdateP1(cpu);
}
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    signal(SIGINT, quit);

    int size;
    int ret = 0;
    int running = 1;
    
    SDL_Window *win = NULL;
    SDL_Renderer *ren = NULL;
    SDL_Texture *tex = NULL;
    SDL_Event event;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }
    win = SDL_CreateWindow("Gameboy Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, LCD_WIDTH * 4, LCD_HEIGHT * 4, SDL_WINDOW_RESIZABLE);
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, LCD_WIDTH, LCD_HEIGHT);
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);

    if (win == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return 1;
    }
    if (ren == NULL) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return 1;
    }
    
    struct CPU cpu;
    struct PPU ppu;

    unsigned char *data = readFile(argv[1], &size);
    for (int i = 0; i < size; i++) {
	cpu.rom[i] = data[i];
    }
    
    init_cpu(&cpu);
    init_ppu(&ppu);
    printf("%d", cpu.rom);
    
    while(running != 0) {
	/* if (cpu.pc == 0) { */
	/*     printf("asdf"); */
	/* } */
	printf("%x - $%x - ", cpu.pc, cpu.memory[cpu.pc]);
	printByteAsBinary(cpu.memory[cpu.pc]);
	putchar(' ');
	ret = parse_opcode(&cpu);
	//output_registers(&cpu);
	if (ret == -1) {
	    // output_memory(&cpu);
	    break;
	}
	cpu.pc += ret;
	cpu.pc++;
	while (SDL_PollEvent(&event)) {
	    if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_q) {
		    running = 0;
		}
		else {
		    for (int j = 0; j < 2*NUM_KEYS; j++)
			if (KEYS[j] == event.key.keysym.sym)
			    {
				KeyPress(&cpu, j%NUM_KEYS);
				break;
			    }
		    
		}
	    }
	    else if (event.type == SDL_KEYUP) {
		for (int j = 0; j < 2*NUM_KEYS; j++)
		    if (KEYS[j] == event.key.keysym.sym)
			{
			    KeyRelease(&cpu, j%NUM_KEYS);
			    break;
			}
	    }
	}

	cpu.clock += 1;
	if (cpu.clock >= 114) {
	    render_frame(&cpu, &ppu, tex, ren);
	    cpu.clock = 0;
	}
	//SDL_Delay(2000);
	handle_interrupts(&cpu);

    }
    putchar('\n');
     output_registers(&cpu);
    output_memory(&cpu);
    // Free the memory
    free(data);
    
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    
    return 0;
}
