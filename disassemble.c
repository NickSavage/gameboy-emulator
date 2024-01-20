#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "opcodes"

static const int REG_B = 0;
static const int REG_C = 1;
static const int REG_D = 2;
static const int REG_E = 3;
static const int REG_H = 4;
static const int REG_L = 5;
static const int REG_F = 6;
static const int REG_A = 7;

static const char *regNames[] = {"b", "c", "d", "e", "h", "l", "f", "a"};

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

int parse_opcode(unsigned char first, unsigned char second, unsigned char third) {
    
    int ret = 0;
    if ((first & 0b11000111) == 0b00000110) {
	// ld r, n
	// load register immediate
	printByteAsBinary(second);
	int reg = (first & 0b00111000) >> 3;
	printf(" ld %s, %d", regNames[reg], second);
	printf(" -- %d -- ", reg);
	ret = 1;
    }
    else if ((first & 0b11000111) == 0b01000110) {
	// ld r, (HL)
	printf(" ld a, [HL]");
    }
    else if ((first & 0b11111000) == 0b01110000) {
	// ld (HL), r
	
	int src = (first & 0b00000111);
	
	printf(" ld [HL], %s", regNames[src]);
    }
    else if ((first & 0b11000000) == 0b01000000) {
	// ld r, r
	// load reg (reg)
	int dest = (first & 0b00111000) >> 3;
	int src = (first & 0b00000111);
	printf(" ld %s, %s", regNames[dest], regNames[src]);
    }
    else if ((first & 0b11111111) == 0b00110110) {
	// ld (HL), n

	printf("ld [HL], %d", second);
	
	ret = 1;
    }
    else if ((first & 0b11111111) == 0b00001010) {
	// ld a, (BC)
	
	printf("ld a, [BC]");
	ret = 1;
    }
    else if ((first & 0b11111000) == 0b10000000) {
	// add r
	int reg = (first & 0b00111000) >> 3;
	printf(" add %s", regNames[reg]);
    }
    else if ((first & 0b11111111) == 0b11000110) {
	// add n
	printByteAsBinary(second);
	putchar(' ');
	printf(" add a, %d", second);

	ret = 1;
    }
    else if ((first & 0b11111000) == 0b10010000) {
	// sub r
	int reg = (first & 0b00111000) >> 3;
 	printf(" sub a, %s", regNames[reg]);
	
	ret = 1;
    }
    else if ((first & 0b11111111) == 0b11010110) {
	// sub n
	printByteAsBinary(second);
	putchar(' ');
	printf(" sub a, %d", second);
	ret = 1;
    }
    else if ((first & 0b11111111) == 0b00100010) {
	// ld (HL+), a
	
	printf(" ld [HL+], a");
    }
    else if ((first & 0b11111111) == 0b11000011) {
	// jp
	int addr = (third << 8) + second;
	printf("JP %d", addr);
	ret = 2;
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
	ret = 2;
	
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
    unsigned char *data;
    int header = 1;

    data = readFile(argv[1], &size);

    char magic[4];
    magic[0] = data[0];
    magic[1] = data[1];
    magic[2] = data[2];
    magic[3] = data[3];
    
    long revision = data[4] | (data[5] << 8) | (data[6] << 16) | (data[7] << 24);
    long symbols = data[8] | (data[9] << 8) | (data[10] << 16) | (data[11] << 24);
    long sections = data[12] | (data[13] << 8) | (data[14] << 16) | (data[15] << 24);
    long nodes = data[16] | (data[17] << 8) | (data[18] << 16) | (data[19] << 24);

    /* printf("%s\n", magic); */
    /* printf("Revision: %ld\n", revision); */
    /* printf("Symbols: %ld\n", symbols); */
    /* printf("Sections: %ld\n", sections); */
    /* printf("Number of nodes: %ld\n", nodes); */
    
    /* char name[4]; */
    /* //    memcpy(name, &data[20], 4); */
    /* name[0] = data[20]; */
    /* name[1] = data[21]; */
    /* name[2] = data[22]; */
    /* name[3] = data[23]; */
    /* printf("%s", name); */
    /* printf("\n\n"); */

    
    /* for (int i = 0; i < size; i++) { */
    /* 	printByteAsBinary(data[i]); */
    /* 	putchar(' '); */
    /* 	i += parse_opcode(data[i], data[i + 1], data[i + 2]); */
	
    /* } */
    for (int i = 0; i < 150; i++) {
	printByteAsBinary(data[0x0100 + i]);
	putchar(' ');
    }
    // Free the memory
    free(data);

    return 0;
}
