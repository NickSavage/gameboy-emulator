// main.cpp
#include <stdio.h>
#include <QApplication>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <cstdlib> // for rand()

QString parse_opcode(int *counter, uint8_t first, uint8_t second, uint8_t third) {
    counter = 0;
    QString ret = "";
    switch (first) {
    case(0xfa):
	*counter = 3;
	ret = "ld a, [a16]";
	
	break;
    default:
	ret = "idk";
    }
    
    if (counter == 0) {
	*counter = 1;
    }
    return ret;
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
    //printf("file size: %d\n", file_size);

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
    QApplication app(argc, argv);

    int size;
    int counter = 0;
    int pc = 0;
    int row = 0;
    unsigned char *data = readFile(argv[1], &size);
    QString hex = "";
    // Create a table widget
    QTableWidget tableWidget(size, 2); // 4 rows, 2 columns
    tableWidget.setHorizontalHeaderLabels({"hex", "Column 2"});

    // Fill the table with random data
    while (pc < 1500) {

	QString instruction = parse_opcode(&counter, data[row], 0, 0);

	printf("counter: %d", counter);
	if (counter == 1) {
	    putchar('a');
	    hex = QString("%1").arg(data[pc]); // Convert to hexadecimal
	} else if (counter == 2) {
	    hex = QString("%1 %2 %3").arg(data[pc], 2, 16, QChar('0'));
	}
	// Generate random data as strings
	QTableWidgetItem *item = new QTableWidgetItem(hex);
	QTableWidgetItem *instr = new QTableWidgetItem(instruction);
	tableWidget.setItem(row, 0, item);
	tableWidget.setItem(row, 1, instr);
	
	pc += counter;
	row += 1;
    }

    // Set table headers to stretch
    tableWidget.horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Show the table
    tableWidget.show();

    return app.exec();
}
