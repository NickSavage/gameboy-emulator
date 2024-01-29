#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <unistd.h>

#include <SDL2/SDL.h>

#include "opcodes.h"

#define LCD_WIDTH 160
#define LCD_HEIGHT 144


static const char *regNames[] = {"b", "c", "d", "e", "h", "l", "f", "a"};
static const char *reg_names_16[] = {"bc", "de", "hl", "sp"};

void quit(int sig) {
    if (sig == SIGINT) {
	exit(0);
    }
}

void init_screen(struct CPU *cpu) {

    cpu->memory[0xFF42] = 0; // store scy in 0xFF42
    cpu->memory[0xFF43] = 0; // store scy in 0xFF42
}

void init_cpu(struct CPU *cpu) {

    cpu->pc = 0x0100;
    cpu->memory[0xFF44] = 144; // plugged, relates to vblank
    for (int i = 0x0000; i < 0x7fff; i++) {
	cpu->memory[i] = cpu->rom[i];
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

void output_memory(struct CPU *cpu) {
    FILE *file = fopen("output.bin", "wb");

    // Write the array to the file
    size_t elements_written = fwrite(cpu->memory, sizeof(int8_t), 65535, file);
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

int parse_opcode(struct CPU *cpu, int pc) {
    
    unsigned char first = cpu->rom[pc];
    unsigned char second = cpu->rom[pc + 1];
    unsigned char third = cpu->rom[pc + 2];
    uint16_t addr;

    int found = 1;
    int ret = 0;
    
    switch (first) {
    case(0x06): case(0x16): case(0x26): case(0x0e): case(0x1e): case(0x2e): case(0x3e): case(0x4e):
	// ld r, n
	printByteAsBinary(second);
	int reg = (first & 0b00111000) >> 3;
	printf(" ld %s, %d", regNames[reg], second);
	load_reg(cpu, reg, second);
	ret = 1;
	break;
	
    case(0xc9):
	// ret

	ret_function(cpu);
	cpu->pc -= 1;

	break;
	    
    case (0x2a):
	// ld a, (HL+)
	addr = (cpu->regs[REG_H] << 8) + cpu->regs[REG_L];
	load_reg(cpu, REG_A, cpu->memory[addr]);

	int total = (cpu->regs[REG_H] << 8) + cpu->regs[REG_L] + 1;
	int high = (total >> 8) & 0xFF;
	int low = total & 0xFF;

	cpu->regs[REG_H] = (total >> 8) & 0xFF;
	cpu->regs[REG_L] = total & 0xFF;
	
	break;
    case(0xcd):
	// call nn
	addr = (third << 8) + second;
	printf(" call [%x]", addr);
	call(cpu, third, second);
	
	cpu->pc = addr - 1;
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
	uint16_t addr = (third << 8) + second;
	printf("%d", addr);
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
    }
    else if ((first & 0b11111111) == 0b00001010) {
	// ld a, (BC)
	
	int addr = (cpu->regs[REG_B] << 8) + cpu->regs[REG_C];
	
	printf(" ld a, [BC]");
	cpu->regs[REG_A] = cpu->memory[addr];
	ret = 1;
    }
    else if ((first & 0b11111111) == 0b00011010) {
	// ld a, (DE)
	
	int addr = (cpu->regs[REG_D] << 8) + cpu->regs[REG_E];
	
	printf(" ld a, [DE]");
	cpu->regs[REG_A] = cpu->memory[addr];
    }
    else if (first == 0b00010010) {
	// ld [DE], a
	int addr = (cpu->regs[REG_D] << 8) + cpu->regs[REG_E];
	
	printf(" ld [DE], a");
	cpu->memory[addr] = cpu->regs[REG_A];
    }
    else if (first == 0b11110000) {
	// ldh a, 0xFF00 + n

	printf(" ldh a, [%x]", 0xFF00 + second);
	cpu->regs[REG_A] = cpu->memory[0xFF00+second];
	ret = 1;
    }
    else if (first == 0b11100000) {
	// ldh 
	printf(" ldh [%x], a", 0xFF00 + second);
	cpu->memory[0xFF00+second] = cpu->regs[REG_A];
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
    else if (first == 0x09) {
	// add hl, de
	printf(" add hl, bc");
	add_16(cpu, 2, (cpu->regs[REG_B] << 8) + cpu->regs[REG_C]);
    }
    else if (first == 0x19) {
	// add hl, de
	printf(" add hl, de");
	add_16(cpu, 2, (cpu->regs[REG_D] << 8) + cpu->regs[REG_E]);
    }
    else if (first == 0x29) {
	// add hl, de
	printf(" add hl, hl");
	add_16(cpu, 2, (cpu->regs[REG_H] << 8) + cpu->regs[REG_L]);
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
    }
    else if ((first & 0b11111111) == 0b00100010) {
	// ld (HL+), a
	int addr = (cpu->regs[REG_H] << 8) + cpu->regs[REG_L];
	
	printf(" ld [HL+], a");
	set_mem(cpu, addr, cpu->regs[REG_A]);
	
	addr += 1;
	cpu->regs[REG_H] = addr >> 8;
	cpu->regs[REG_L] = addr & 0xFF;
	
    }
    else if (first == 0x13) {
	printf("inc de");
	int total = (cpu->regs[REG_D] << 8) + cpu->regs[REG_E] + 1;
	int high = (total >> 8) & 0xFF;
	int low = total & 0xFF;

	cpu->regs[REG_D] = high;
	cpu->regs[REG_E] = low;

	printf("; %x", (cpu->regs[REG_D] << 8) + cpu->regs[REG_E]);

    }
    else if (first == 0b00100011) {
	uint8_t reg = (first >> 4) & 0b0011;
	printf("inc hl");

	int total = (cpu->regs[REG_H] << 8) + cpu->regs[REG_L] + 1;
	int high = (total >> 8) & 0xFF;
	int low = total & 0xFF;

	cpu->regs[REG_H] = high;
	cpu->regs[REG_L] = low;
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
    }
    else if ((first & 0b11111111) == 0b11000011) {
	// jp
	int addr = (third << 8) + second;
	printf(" JP %d", addr);
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
	} else if (cc == COND_C && (check_flag_c(cpu) == 1)) {
	    cpu->pc = addr - 1;
	} else {
	    
	    ret = 2;
	}
	
    }
    else if ((first & 0b11111111) == 0b00011000) {
	// jr e
	printf(" jr %d", (int8_t)second);
	cpu->pc += (int8_t)second + 1;
	//	ret = -1;
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
	} else if (cc == COND_NZ && (get_z_flag(cpu) == 0)) {
	    printf("does htis get triggered?");
	    cpu->pc += (int8_t)second;
	} else {
	    ret = 1;
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
    }
    else if (first == 0xC1) {
	// pop rr
	printf(" pop bc");
	pop(cpu, 0);
    }
    else if (first == 0xD1) {
	// pop rr
	printf(" pop de");
	pop(cpu, 1);
    }
    else if (first == 0xE1) {
	// pop rr
	printf(" pop hl");
	pop(cpu, 2);
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


void build_fb(struct CPU *cpu, uint32_t (*fb)[LCD_WIDTH]) {
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
    for (uint8_t ly = 0; ly < 153; ly++) {
	cpu->memory[0xFF44] = ly; // store ly in 0xFF44
	if (ly >= 144) {
	    continue; // vblank
	}
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

	    fb[ly][x] = colour_pixel;
	    //   printf("ly: %d, x: %d, tile_x: %d, tile_y: %d, tile_index: %d, pixel: %d\n", ly, x, tile_x, tile_y, tile_index, pixel);
	}
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    signal(SIGINT, quit);

    size_t size;
    int ret = 0;
    int running = 1;
    
    SDL_Window *win = NULL;
    SDL_Renderer *ren = NULL;
    SDL_Texture *tex = NULL;
    SDL_Event event;
    uint32_t fb[LCD_HEIGHT][LCD_WIDTH];
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
    printf("%d", cpu.rom);
    
    while(running != 0) {
	printf("%x - ", cpu.pc);
	printByteAsBinary(cpu.rom[cpu.pc]);
	putchar(' ');
	ret = parse_opcode(&cpu, cpu.pc);
	if (ret == -1) {
	    output_memory(&cpu);
	    break;
	}
	cpu.pc += ret;
	cpu.pc++;
	/* while (SDL_PollEvent(&event)) { */
	/*     if (event.key.keysym.sym == SDLK_q) { */
	/* 	running = 0; */
	/*     } */
	/* } */

	if ((cpu.memory[0xFF40] & 0b10000000) >> 7 == 1) {
	    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
	    SDL_RenderClear(ren);

	    build_fb(&cpu, fb);
	    //	    SDL_UpdateTexture(tex, NULL, fb, LCD_WIDTH * sizeof(uint32_t));
	    SDL_UpdateTexture(tex, NULL, fb, LCD_WIDTH * 4);
	    SDL_RenderClear(ren);
	    SDL_RenderCopy(ren, tex, NULL, NULL);
	    SDL_RenderPresent(ren);
	}
	//SDL_Delay(2000);

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
    
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    
    return 0;
}
