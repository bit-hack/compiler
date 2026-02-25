#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>


#define ERROR(...) { \
  printf("Error, line %u: ", lex_line_num()); \
  printf(__VA_ARGS__); \
  printf("\n"); \
  exit(1); \
}

#define TRACE(FUNC) { \
  printf("%s\n", FUNC); \
}

typedef enum {
  TOK_UNKNOWN,
  TOK_SEMICOLON,
  TOK_LPAREN,
  TOK_RPAREN,
  TOK_LBRACE,
  TOK_RBRACE,
  TOK_IDENT,
  TOK_INT,
  TOK_INT_LIT,
  TOK_VOID,
  TOK_RETURN,
  TOK_COMMA,
  TOK_ASSIGN,
  TOK_ADD,
  TOK_SUB,
  TOK_MUL,
  TOK_EOF,
} token_type_t;

typedef struct {
  const char *start;
  const char *end;
  token_type_t type;
  uint32_t line;
} token_t;

typedef struct {
  const char *start;
  const char *end;
  const char *ptr;
  const char *line_start;
  uint32_t line_num;
} lex_t;

const char *tok_name(token_type_t type);
bool tok_is_type(token_t *t);
bool tok_is_operator(token_t *t);
bool tok_is(token_t *t, token_type_t type);
int tok_precedence(token_t *t);

bool lex_init(const char *file);
void lex_pop(token_t *out);
void lex_peek(token_t *out);
void lex_expect(token_type_t type);
bool lex_found(token_type_t type, token_t *out);
uint32_t lex_line_num(void);

void parse(void);
