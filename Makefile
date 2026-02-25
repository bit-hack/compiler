all:
	gcc -g -O0 token.c lex.c parse.c main.c -o compiler

test:
	./compiler tests/test.c
