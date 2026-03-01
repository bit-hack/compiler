#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#define ERROR(...) { \
  printf("Error, line %u: ", lLineNum()); \
  printf(__VA_ARGS__); \
  printf("\n"); \
  exit(1); \
}

#define ERROR_LN(LN, ...) { \
  printf("Error, line %u: ", LN); \
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
  TOK_CHAR,
  TOK_SHORT,
  TOK_INT,
  TOK_INT_LIT,
  TOK_IF,
  TOK_ELSE,
  TOK_VOID,
  TOK_RETURN,
  TOK_BREAK,
  TOK_CONTINUE,
  TOK_WHILE,
  TOK_DO,
  TOK_FOR,
  TOK_COMMA,    // ,
  TOK_ASSIGN,   // =
  TOK_BIT_AND,  // &
  TOK_LOG_AND,  // &&
  TOK_BIT_OR,   // |
  TOK_LOG_OR,   // ||
  TOK_BIT_XOR,  // ^
  TOK_BIT_NOT,  // ~
  TOK_LOG_NOT,  // !
  TOK_SHL,      // <<
  TOK_SHR,      // >>
  TOK_NEQ,      // !=
  TOK_EQ,       // ==
  TOK_LT,       // <
  TOK_LTE,      // <=
  TOK_GT,       // >
  TOK_GTE,      // >=
  TOK_ADD,      // +
  TOK_SUB,      // -
  TOK_MUL,      // *
  TOK_DIV,      // /
  TOK_MOD,      // %
  TOK_EOF,
} token_type_t;

typedef enum {
  AST_ROOT,
  AST_DECL_TYPE,
  AST_DECL_VAR,
  AST_DECL_FUNC,
  AST_STMT_RETURN,
  AST_STMT_EXPR,
  AST_STMT_COMPOUND,
  AST_STMT_IF,
  AST_STMT_WHILE,
  AST_STMT_BREAK,
  AST_STMT_CONTINUE,
  AST_STMT_DO,
  AST_STMT_FOR,
  AST_EXPR_IDENT,
  AST_EXPR_INT_LIT,
  AST_EXPR_BIN_OP,
  AST_EXPR_UNARY_OP,
  AST_EXPR_CALL,
  AST_EXPR_CAST,
} ast_node_type_t;

typedef struct {
  const char*  start;
  const char*  end;
  token_type_t type;
  uint32_t     line;
} token_t;

typedef struct {
  const char *start;
  const char *end;
  const char *ptr;
  const char *lineStart;
  uint32_t    lineNum;
} lex_t;

typedef struct ast_node_s ast_node_t, *ast_node_p;

typedef struct {
  ast_node_p exprStack[1024];
  size_t     exprStackHead;
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
      token_t token;
    } declType;

    struct {
      ast_node_p type;
      token_t    ident;
      ast_node_p args;
      ast_node_p body;
    } declFunc;

    struct {
      ast_node_p type;
      token_t    ident;
      ast_node_p expr;
    } declVar;

    struct {
      ast_node_p expr;
    } stmtExpr;

    struct {
      token_t    token;
      ast_node_p expr;
    } stmtReturn;

    struct {
      token_t    token;
    } stmtBreak;

    struct {
      token_t    token;
    } stmtContinue;

    struct {
      ast_node_p stmt;
    } stmtCompound;

    struct {
      token_t    token;
      ast_node_p expr;
      ast_node_p isTrue;
      ast_node_p isFalse;
    } stmtIf;

    struct {
      token_t    token;
      ast_node_p expr;
      ast_node_p body;
    } stmtWhile;

    struct {
      token_t    token;
      ast_node_p expr;
      ast_node_p body;
    } stmtDo;

    struct {
      token_t    token;
      ast_node_p init;
      ast_node_p cond;
      ast_node_p update;
      ast_node_p body;
    } stmtFor;

    struct {
      token_t token;
    } exprIdent;

    struct {
      token_t token;
    } exprIntLit;

    struct {
      token_t    op;
      ast_node_p lhs;
      ast_node_p rhs;
    } exprBinOp;

    struct {
      token_t    op;
      ast_node_p rhs;
    } exprUnaryOp;

    struct {
      token_t    ident;
      ast_node_p arg;
    } exprCall;

    struct {
      ast_node_p type;
      ast_node_p expr;
    } exprCast;
  };

} ast_node_t;


const char* tTypeName  (token_type_t type);
const char *tName      (const token_t *t);
bool        tIsType    (const token_t *t);
bool        tIsOperator(const token_t *t);
bool        tIs        (const token_t *t, token_type_t type);
int         tPrec      (const token_t *t);
bool        tEqual     (const token_t* a, const token_t* b);
int         tSize      (const token_t* t);
int         tLineNum   (const token_t* t);

bool        lInit      (const char *file);
void        lPop       (token_t *out);
void        lPeek      (token_t *out);
void        lExpect    (token_type_t type, token_t *out);
bool        lFound     (token_type_t type, token_t *out);
uint32_t    lLineNum   (void);

ast_node_p  pParse     (void);

ast_node_p  aNodeNew   (ast_node_type_t type);
ast_node_p  aNodeInsert(ast_node_p chain, ast_node_p toInsert);
void        aDump      (ast_node_p n);

void        sCheck     (ast_node_p n);
