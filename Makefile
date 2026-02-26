all:
	gcc -g -O0 token.c lexer.c parser.c ast.c main.c -o compiler

test:
	./compiler tests/test.c
