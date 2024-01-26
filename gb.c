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

int16_t ppc;

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
    cpu->pc = 36;
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

	printf("ld %s%s, %x", regNames[reg + reg] , regNames[reg + reg + 1], (high << 8) + low);
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
    else if ((first & 0b11111111) == 0b00011000) {
	// jr e
	printf(" jr %d", (int8_t)second);
	ret = -1;
    }
    else if ((first & 0b11100111) == 0b00100000) {
	// jr cc, nn
	printByteAsBinary(second);
	putchar(' ');
	
	int cc = (first & 0b00011000) >> 3;
	printf(" JR %d, %d", cc, (int8_t)second);

	ret = 1;
	if (cc == COND_C && (check_flag_c(cpu) == 1)) {
	    ppc += (int8_t)second;
	} else if (cc == COND_NZ && (get_z_flag(cpu) == 0)) {
	    ppc += (int8_t)second;
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
    putchar('\n');
    return ret;
}

uint8_t bg_tile_map_mode(struct CPU *cpu) {
    uint8_t byte = cpu->memory[0xFF40];
    uint8_t result = (byte & 0b00001000) << 3;
    return result;
}

uint32_t colour_tile(int input) {
    uint32_t result = 0;
    if (input > 0) {
	result = 0xFFFFFFFF;
    } else {
	result = 0x00000000;
    }
    //    printf(", %x\n", result);
    return result;
}

/* void put_tile(uint32_t (*fb)[LCD_WIDTH], uint16_t tile, int x_offset, int y_offset) { */
/*     printf("%x, %d-%d", tile, x_offset, y_offset); */
/*     fb[x_offset][y_offset + 0] = colour_tile((tile >> 14) & 0x3); */
/*     fb[x_offset][y_offset + 1] = colour_tile((tile >> 12) & 0x3); */
/*     fb[x_offset][y_offset + 2] = colour_tile((tile >> 10) & 0x3); */
/*     fb[x_offset][y_offset + 3] = colour_tile((tile >> 8) & 0x3); */
/*     fb[x_offset][y_offset + 4] = colour_tile((tile >> 6) & 0x3); */
/*     fb[x_offset][y_offset + 5] = colour_tile((tile >> 4) & 0x3); */
/*     fb[x_offset][y_offset + 6] = colour_tile((tile >> 2) & 0x3); */
/*     fb[x_offset][y_offset + 7] = colour_tile((tile >> 0) & 0x3); */
/* } */
void put_tile(uint32_t (*fb)[LCD_WIDTH], uint16_t tile, int x_offset, int y_offset) {
    printf("%x, %d-%d", tile, x_offset, y_offset);
    fb[y_offset + 0][x_offset] = colour_tile((tile >> 14) & 0x3);
    fb[y_offset + 1][x_offset] = colour_tile((tile >> 12) & 0x3);
    fb[y_offset + 2][x_offset] = colour_tile((tile >> 10) & 0x3);
    fb[y_offset + 3][x_offset] = colour_tile((tile >> 8) & 0x3);
    fb[y_offset + 4][x_offset] = colour_tile((tile >> 6) & 0x3);
    fb[y_offset + 5][x_offset] = colour_tile((tile >> 4) & 0x3);
    fb[y_offset + 6][x_offset] = colour_tile((tile >> 2) & 0x3);
    fb[y_offset + 7][x_offset] = colour_tile((tile >> 0) & 0x3);
}

// todo: this needs to build tiles first and then put them on the screen, instead of doing it on the fly like this
void build_fb(struct CPU *cpu, uint32_t (*fb)[LCD_WIDTH]) {
    uint8_t bg_tile_map_mode_addr = bg_tile_map_mode(cpu);
    uint16_t addr;
    uint16_t tile;
    uint8_t pixel;
    uint8_t x_offset = 0;
    
    uint8_t tile_x;
    uint8_t tile_y;
    uint8_t tile_pixel_x;
    uint8_t tile_pixel_y;
    uint8_t tile_index;
    uint8_t tile_id;
/*     for (int x = 0; x < LCD_WIDTH; x++) { */
/* 	fb[x][0] = 0xFFFF; */
/* 	fb[x][1] = 0xFFFF; */
/*     } */
/* } */
    for (uint8_t ly = 0; ly < 153; ly++) {
	cpu->memory[0xFF44] = ly; // store ly in 0xFF44
	if (ly >= 144) {
	    continue;
	}
	for (uint8_t x = 0; x <= 160; x++) {
	    
	    tile_x = x / 8;
	    tile_y = ly / 8;
	    tile_pixel_x = x % 8;
	    tile_pixel_y = ly % 8;

	    printf("%d, %d, %d, %d\n", ly, x, tile_y, tile_x);
	    tile_index = tile_y * 32 + tile_x;
	    //	    addr = 0x9800 + tile_index;
	    tile_id = cpu->memory[0x9800 + tile_index]; 
	    addr = 0x9000 + tile_id * 16 + tile_pixel_y * 2;
	    printf("tile_index: %d, tile_id: %d, addr: %x\n", tile_index, tile_id, addr);
	    /* if (bg_tile_map_mode_addr == 1) { */
	    /* 	addr = 0x9000 + ((i) * 16); */
	    /* } else { */
	    //	    addr = 0x9000 + (tile_y * 0) + (tile_x);
		//}
	    tile = interleave_tile(cpu->memory[addr], cpu->memory[addr + 1]);
	    //  pixel = colour_tile((tile >> (tile_x % 8)) & 0x3);
	    fb[ly][x] = tile;

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

    tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, LCD_WIDTH, LCD_HEIGHT);

    if (win == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return 1;
    }
    if (ren == NULL) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return 1;
    }
    
    SDL_RenderSetLogicalSize(ren, LCD_WIDTH, LCD_HEIGHT);
    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
    SDL_RenderClear(ren);
    SDL_RenderPresent(ren);
    struct CPU cpu;
    struct PPU ppu;

    unsigned char *data = readFile(argv[1], &size);
    for (int i = 0; i < size; i++) {
	cpu.rom[i] = data[i];
    }
    ppc = 0x0100;
    
    init_cpu(&cpu);
    printf("%d", cpu.rom);
    
    while(running != 0) {
	printf("%x - ", ppc);
	printByteAsBinary(cpu.rom[ppc]);
	putchar(' ');
	ret = parse_opcode(&cpu, ppc);
	/* if (ret == -1) { */
	/*     output_memory(&cpu); */
	/*     break; */
	/* } */
	ppc += ret;
	ppc++;
	/* while (SDL_PollEvent(&event)) { */
	/*     if (event.key.keysym.sym == SDLK_q) { */
	/* 	running = 0; */
	/*     } */
	/* } */

	if ((cpu.memory[0xFF40] & 0b10000000) >> 7 == 1) {
	    build_fb(&cpu, fb);
	    SDL_UpdateTexture(tex, NULL, fb, LCD_WIDTH * sizeof(uint32_t));
	    //SDL_UpdateTexture(tex, NULL, fb, LCD_WIDTH);
	    SDL_RenderClear(ren);
	    SDL_RenderCopy(ren, tex, NULL, NULL);
	    SDL_RenderPresent(ren);
	}
	//SDL_Delay(2000);

    }
    build_fb(&cpu, fb);
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
