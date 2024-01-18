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

static const char *regNames[] = {"b", "c", "d", "e", "h", "l", "f", "a"};
int8_t memory[65535];
int8_t regs[] = {0,0,0,0,0,0,0,0};
int16_t pc = 0;

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

void sub(unsigned char amount) {
    regs[REG_A] -= amount;
    
    if (regs[REG_A] == 0) {
	regs[REG_F] |= 1 << FLAG_Z;
    } else {
	regs[REG_F] |= 0 << FLAG_Z;
    }
    regs[REG_F] |= 1 << FLAG_N;
    
}

int parse_opcode(unsigned char first, unsigned char second, unsigned char third) {
    
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
	regs[REG_A] = memory[addr];
    }
    else if ((first & 0b11111000) == 0b01110000) {
	// ld (HL), r
	
	int addr = (regs[REG_H] << 8) + regs[REG_L];
	int src = (first & 0b00000111);
	
	printf(" ld [HL], %s", regNames[src]);

	memory[addr] = regs[src];
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
	
	memory[addr] = second;
	ret = 1;
    }
    else if ((first & 0b11111111) == 0b00001010) {
	// ld a, (BC)
	
	int addr = (regs[REG_B] << 8) + regs[REG_C];
	
	regs[REG_A] = memory[addr];
	ret = 1;
    }
    else if ((first & 0b11111000) == 0b10000000) {
	// add r
	int reg = (first & 0b00111000) >> 3;
	printf(" add %s", regNames[reg]);
	
	regs[REG_A] += regs[reg];

	regs[REG_F] |= 0 << FLAG_N;
	
    }
    else if ((first & 0b11111111) == 0b11000110) {
	// add n
	printByteAsBinary(second);
	putchar(' ');
	printf(" add a, %d", second);
	regs[REG_A] += second;
	printf("-%d-", regs[REG_A]);
	ret = 1;

	regs[REG_F] |= 0 << FLAG_N;
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
	memory[addr] = regs[REG_A];
	
	addr -= 1;
	regs[REG_H] = addr >> 8;
	regs[REG_L] = addr & 0xFF;
	
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
    unsigned char *fileData;

    int header = 0;

    fileData = readFile(argv[1], &size);

    // Print each byte in the buffer
    for (long i = 0; i < size; i++) {

	unsigned char byte = fileData[i];
	printByteAsBinary(byte);
	putchar(' ');
	if (i == 0) {
	    printByteAsBinary(fileData[i + 1]);
	    putchar(' ');
	    printByteAsBinary(fileData[i + 2]);
	    putchar(' ');
	    printByteAsBinary(fileData[i + 3]);
	    putchar(' ');
	    printf("%c%c%c%c", fileData[i], fileData[i +1], fileData[i + 2], fileData[i + 3]);
	    i += 3;
	}
	if (i == 4) {
	    printByteAsBinary(fileData[i + 1]);
	    putchar(' ');
	    printByteAsBinary(fileData[i + 2]);
	    putchar(' ');
	    printByteAsBinary(fileData[i + 3]);
	    putchar(' ');
	    printf("Revision Number");
	    i += 3;
	}
	if (i == 8) {
	    printByteAsBinary(fileData[i + 1]);
	    putchar(' ');
	    printByteAsBinary(fileData[i + 2]);
	    putchar(' ');
	    printByteAsBinary(fileData[i + 3]);
	    putchar(' ');
	    printf("Number of Symbols");
	    i += 3;
	}
	if (i == 12) {
	    printByteAsBinary(fileData[i + 1]);
	    putchar(' ');
	    printByteAsBinary(fileData[i + 2]);
	    putchar(' ');
	    printByteAsBinary(fileData[i + 3]);
	    putchar(' ');
	    printf("Number of Sections");
	    i += 3;
	    header = 1;
	}
	if (i > 15 && header == 1) {
	    printByteAsBinary(fileData[i + 1]);
	    putchar(' ');
	    printByteAsBinary(fileData[i + 2]);
	    putchar(' ');
	    printByteAsBinary(fileData[i + 3]);
	    putchar(' ');
	    printf("Number of Nodes");
	    i += 3;
	    header = 0;
	}

	i += parse_opcode(fileData[i], fileData[i + 1], fileData[i + 2]);
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
    free(fileData);

    return 0;
}
