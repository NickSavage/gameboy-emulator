target = build test

build:
	gcc -g gb.c opcodes.c -lSDL2 -o gb

disassemble:
	gcc -g disassemble.c opcodes.c -o disassemble

test:
	gcc -g test_gb.c opcodes.c -o test_gb
	chmod +x test_gb
	./test_gb
clean:
	rm test_gb
	rm gb
