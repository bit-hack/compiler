#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


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
  TOK_IF,
  TOK_ELSE,
  TOK_VOID,
  TOK_RETURN,
  TOK_BREAK,
  TOK_CONTINUE,
  TOK_WHILE,
  TOK_COMMA,
  TOK_ASSIGN,
  TOK_BIT_AND,
  TOK_LOG_AND,
  TOK_BIT_OR,
  TOK_LOG_OR,
  TOK_BIT_XOR,
  TOK_ADD,
  TOK_SUB,
  TOK_MUL,
  TOK_DIV,
  TOK_MOD,
  TOK_EOF,
} token_type_t;

typedef enum {
  AST_ROOT,
  AST_DECL_VAR,
  AST_DECL_FUNC,
  AST_STMT_RETURN,
  AST_STMT_EXPR,
  AST_STMT_COMPOUND,
  AST_STMT_IF,
  AST_STMT_WHILE,
  AST_STMT_BREAK,
  AST_STMT_CONTINUE,
  AST_EXPR_IDENT,
  AST_EXPR_INT_LIT,
  AST_EXPR_BIN_OP,
} ast_node_type_t;

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

typedef struct ast_node_s ast_node_t, *ast_node_p;

typedef struct {
  ast_node_p exprStack[1024];
  size_t exprStackHead;

  ast_node_p astRoot;
} parser_t;

typedef struct ast_node_s {

  ast_node_type_t type;

  ast_node_p last;
  ast_node_p next;

  union {

    struct {
      ast_node_p node;
    } root;

    struct {
      token_t type;
      token_t ident;
      ast_node_p args;
      ast_node_p body;
    } decl_func;

    struct {
      token_t type;
      token_t ident;
      ast_node_p expr;
    } decl_var;

    struct {
      ast_node_p expr;
    } stmt_expr;

    struct {
      ast_node_p expr;
    } stmt_return;

    struct {
      ast_node_p stmt;
    } stmt_compound;

    struct {
      ast_node_p expr;
      ast_node_p is_true;
      ast_node_p is_false;
    } stmt_if;

    struct {
      ast_node_p expr;
      ast_node_p body;
    } stmt_while;

    struct {
      token_t token;
    } expr_ident;

    struct {
      token_t token;
    } expr_int_lit;

    struct {
      token_t op;
      ast_node_p lhs;
      ast_node_p rhs;
    } expr_bin_op;

  };

} ast_node_t;

const char *tok_name(token_type_t type);
bool tok_is_type(token_t *t);
bool tok_is_operator(token_t *t);
bool tok_is(token_t *t, token_type_t type);
int tok_prec(token_t *t);
void tok_print(token_t *t);

bool lex_init(const char *file);
void lex_pop(token_t *out);
void lex_peek(token_t *out);
void lex_expect(token_type_t type);
bool lex_found(token_type_t type, token_t *out);
uint32_t lex_line_num(void);

void parse(void);

ast_node_p ast_node_new(ast_node_type_t type);
ast_node_p ast_node_insert(ast_node_p chain, ast_node_p to_insert);
void ast_walk(ast_node_p n, int indent);
