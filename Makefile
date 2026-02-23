all:
	gcc -g -O0 compiler.c -o compiler

test:
	./compiler test.c
