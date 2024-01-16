#include <stdio.h>
#include <stdlib.h>

static const char *regNames[] = {"b", "c", "d", "e", "h", "l", "[HL]", "a"};

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
	if ((byte & 0b11000111) == 0b00000110) {
	    // load register immediate
	    printByteAsBinary(fileData[i + 1]);
	    int reg = (byte & 0b00111000) >> 3;
	    printf(" ld %s, %d", regNames[reg], fileData[i + 1]);
	    i++;
	}
	if ((fileData[i] & 0b11000000) == 0b01000000) {
	    // load reg (reg)
	    int dest = (byte & 0b00111000) >> 3;
	    int src = (byte & 0b00000111);
	    printf(" ld %s, %s", regNames[dest], regNames[src]);
	    
	}
	putchar('\n');
    }
    putchar('\n');

    // Free the memory
    free(fileData);

    return 0;
}
