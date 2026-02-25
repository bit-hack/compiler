#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define TRACE(FUNC) { printf("%s\n", FUNC); }

#define ERROR(...) { \
  printf("Error, line %u: ", lex.line_num); \
  printf(__VA_ARGS__); \
  printf("\n"); \
  exit(1); \
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

static lex_t lex;

static void parse_expr(int minPrec);

static bool lex_init(const char *file) {

  FILE *fd = fopen(file, "r");
  if (!fd) {
    printf("Unable to open '%s'\n", file);
    return false;
  }

  // file size
  fseek(fd, 0, SEEK_END);
  const long fd_size = ftell(fd);
  fseek(fd, 0, SEEK_SET);

  // read file data
  char *src = malloc(fd_size + 1);
  fread(src, 1, fd_size, fd);
  src[fd_size] == '\0';

  // close file handle
  fclose(fd);

  // init lexer
  lex.start = src;
  lex.end = src + fd_size;
  lex.line_num = 1;
  lex.line_start = src;
  lex.ptr = src;

  return true;
}

static void lex_skip_whitespace(void) {
  const char *p = lex.ptr;
  for (;*p; ++p) {

    // TODO: handle comments...

    switch (*p) {
    case '\n':
      lex.line_num++;
      lex.line_start = (p+1);
    case ' ':
    case '\t':
    case '\r':
      continue;
    }
    break;
  }
  lex.ptr = p;
}

static bool lex_is_alpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool lex_is_numeric(char c) {
  return (c >= '0' && c <= '9');
}

static bool lex_match(const char *s) {
  const char *p = lex.ptr;
  for (;; ++p, ++s) {
    if (*s == '\0') {
      lex.ptr = p;
      return true;
    }
    if (*s != *p) {
      return false;
    }
  }
}

static bool lex_identifier(token_t *out) {
  const char *p = lex.ptr;
  if (!lex_is_alpha(*p) && *p != '_') {
    return false;
  }
  for (++p;;++p) {
    if (lex_is_alpha(*p)) {
      continue;
    }
    if (lex_is_numeric(*p)) {
      continue;
    }
    if (*p == '_') {
      continue;
    }
    break;
  }
  out->end = p;
  lex.ptr = p;
  return true;
}

static bool lex_int_literal(token_t *out) {
  const char *p = lex.ptr;
  for (; lex_is_numeric(*p); ++p);
  if (lex.ptr == p) {
    return false;
  }
  out->end = p;
  lex.ptr = p;
  return true;
}

static void lex_pop(token_t *out) {

  lex_skip_whitespace();

  // prepare outgoing token
  out->type  = TOK_UNKNOWN;
  out->line  = lex.line_num;
  out->start = lex.ptr;
  out->end   = NULL;

  // first stage simple classifier
  switch (*lex.ptr) {
  case '\0': out->type = TOK_EOF;       break;
  case ';':  out->type = TOK_SEMICOLON; break;
  case '(':  out->type = TOK_LPAREN;    break;
  case ')':  out->type = TOK_RPAREN;    break;
  case '{':  out->type = TOK_LBRACE;    break;
  case '}':  out->type = TOK_RBRACE;    break;
  case '=':  out->type = TOK_ASSIGN;    break;
  case '+':  out->type = TOK_ADD;       break;
  case '-':  out->type = TOK_SUB;       break;
  case '*':  out->type = TOK_MUL;       break;
  case 'v': if (lex_match("void"))   { out->type = TOK_VOID;   } break;
  case 'i': if (lex_match("int"))    { out->type = TOK_INT;    } break;
  case 'r': if (lex_match("return")) { out->type = TOK_RETURN; } break;
  }

  // second stage classifier
  if (out->type == TOK_UNKNOWN) {
    do {
      if (lex_identifier(out))  { out->type = TOK_IDENT;   break; }
      if (lex_int_literal(out)) { out->type = TOK_INT_LIT; break; }

      ERROR("unknown token on linr %u", lex.line_num);

    } while (0);
  }

  // fixup for one character tokens
  if (lex.ptr == out->start) {
    ++lex.ptr;
  }

  // fill in token end
  out->end = lex.ptr;

#if 1
  const int size = out->end - out->start;
  printf("<%.*s>\n", size, out->start);
#endif
}

static void lex_peek(token_t *out) {
  const lex_t save = lex;
  lex_pop(out);
  lex = save;
}

static void lex_expect(token_type_t type) {
  token_t t;
  lex_pop(&t);
  if (t.type != type) {
    ERROR("expected token '%u'", type);
  }
}

static bool lex_found(token_type_t type, token_t *out) {

  token_t temp;
  out = out ? out : &temp;

  lex_peek(out);
  if (out->type == type) {
    lex_pop(out);
    return true;
  }
  return false;
}

static bool tok_is_type(token_t *t) {
  switch (t->type) {
  case TOK_VOID:
  case TOK_INT:
    return true;
  default:
    return false;
  }
}

static bool tok_is(token_t *t, token_type_t type) {
  return t->type == type;
}

static bool tok_is_operator(token_t *t) {
  switch (t->type) {
  case TOK_ADD:
  case TOK_SUB:
  case TOK_MUL:
  case TOK_ASSIGN:
    return true;
  default:
    return false;
  }
}

static void parse_expr_primary(void) {

  token_t prim;
  lex_pop(&prim);

  // parenthesized expression
  if (tok_is(&prim, TOK_LPAREN)) {
    parse_expr(/*minPrec=*/0);
    lex_expect(TOK_RPAREN);
    return;
  }

  if (tok_is(&prim, TOK_IDENT)) {
    return;
  }

  if (tok_is(&prim, TOK_INT_LIT)) {
    return;
  }

  ERROR("Primary expression expected");
}

static int tok_precedence(token_t *t) {
  switch (t->type) {
  case TOK_ASSIGN:
    return 1;
  case TOK_ADD:
  case TOK_SUB:
    return 8;
  case TOK_MUL:
//case TOK_MOD:
//case TOK_DIV:
    return 9;
  default:
    ERROR("Internal error");
  }
}

static bool parse_check_precedence(token_t *t, int minPrec) {
  if (tok_is(t, TOK_ASSIGN)) {
    return tok_precedence(t) < minPrec;
  }
  else {
    return tok_precedence(t) <= minPrec;
  }
}

static void parse_expr(int minPrec) {

  // lhs
  parse_expr_primary();

  for (;;) {

    // look for an operator
    token_t op;
    lex_peek(&op);
    if (!tok_is_operator(&op)) {
      break;
    }

    // if our found operator binds less tightly than a prior one stop.
    if (parse_check_precedence(&op, minPrec)) {
      break;
    }

    // pop the operator
    lex_pop(&op);

    // rhs
    parse_expr(tok_precedence(&op));
  }
}

static void parse_stmt_return(void) {

  if (lex_found(TOK_SEMICOLON, NULL)) {
    return;
  }

  parse_expr(/*minPrec=*/0);
  lex_expect(TOK_SEMICOLON);
}

static void parse_stmt_local_decl(void) {

  token_t type;
  lex_pop(&type);

  token_t ident;
  lex_pop(&ident);

  if (lex_found(TOK_ASSIGN, NULL)) {
    parse_expr(/*minPrec=*/0);
  }

  lex_expect(TOK_SEMICOLON);
}

static void parse_stmt(void) {

  token_t la;
  lex_peek(&la);

  if (tok_is_type(&la)) {
    parse_stmt_local_decl();
    return;
  }

  if (tok_is(&la, TOK_RETURN)) {
    lex_pop(&la);
    parse_stmt_return();
    return;
  }

  // fallback to trying to parse an expression
  parse_expr(/*minPrec=*/0);
  lex_expect(TOK_SEMICOLON);
}

static void parse_func(token_t *type, token_t *ident) {

  // parse arguments
  if (!lex_found(TOK_RPAREN, NULL)) {
    do {

      token_t arg_type;
      lex_pop(&arg_type);
      if (tok_is(&arg_type, TOK_VOID)) {
        break;
      }

      token_t arg_ident;
      lex_pop(&arg_ident);

    } while (lex_found(TOK_COMMA, NULL));
  }
  lex_expect(TOK_RPAREN);

  // parse function body
  lex_expect(TOK_LBRACE);
  while (!lex_found(TOK_RBRACE, NULL)) {

    parse_stmt();
  }
}

static void parse(void) {

  token_t token;
  while (!lex_found(TOK_EOF, &token)) {

    token_t type;
    lex_pop(&type);

    token_t ident;
    lex_pop(&type);

    lex_expect(TOK_LPAREN);
    parse_func(&type, &ident);
  }
}

int main(int argc, char **args) {

  if (argc <= 1) {
    printf("usage: %s <file.c>\n", args[0]);
    return 0;
  }

  lex_init(args[1]);

  parse();

  return 0;
}
