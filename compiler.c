#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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

typedef struct {
  // global
} parser_t;

lex_t lex;

#define error(...) { \
  printf(__VA_ARGS__); \
  printf("\n"); \
  exit(1); \
}

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
  if (*p == '-') {
    ++p;
  }
  for (; lex_is_numeric(*p); ++p);
  if (lex.ptr == p) {
    return false;
  }
  out->end = p;
  lex.ptr = p;
  return true;
}

static void lex_peek(token_t *out) {
  lex_skip_whitespace();
  out->type  = TOK_UNKNOWN;
  out->line  = lex.line_num;
  out->start = lex.ptr;
  out->end   = NULL;

  switch (*lex.ptr) {
  case '\0': out->type = TOK_EOF;       break;
  case ';':  out->type = TOK_SEMICOLON; break;
  case '(':  out->type = TOK_LPAREN;    break;
  case ')':  out->type = TOK_RPAREN;    break;
  case '{':  out->type = TOK_LBRACE;    break;
  case '}':  out->type = TOK_RBRACE;    break;
  case 'v': if (lex_match("void"))   { out->type = TOK_VOID;   } break;
  case 'i': if (lex_match("int"))    { out->type = TOK_INT;    } break;
  case 'r': if (lex_match("return")) { out->type = TOK_RETURN; } break;
  }

  if (out->type == TOK_UNKNOWN) {
    do {
      if (lex_identifier(out))  { out->type = TOK_IDENT;   break; }
      if (lex_int_literal(out)) { out->type = TOK_INT_LIT; break; }

      error("unknown token");

    } while (0);
  }

  out->end = lex.ptr;
  if (lex.ptr == out->start) {
    out->end = ++lex.ptr;
  }
}

static void lex_pop(token_t *out) {
  lex_peek(out);
  lex.ptr = out->end;
}

static void lex_expect(token_type_t type) {
  token_t t;
  lex_pop(&t);
  if (t.type != type) {
    // ERROR
  }
}

static bool lex_found(token_type_t type, token_t *out) {
  lex_peek(out);
  if (out->type == type) {
    lex.ptr  = out->end;
    return true;
  }
  return false;
}

static bool tok_is_type(token_type_t t) {
  switch (t) {
  case TOK_VOID:
  case TOK_INT:
    return true;
  default:
    return false;
  }
}

static bool parse() {
  return false;
}

int main(int argc, char **args) {

  if (argc <= 1) {
    printf("usage: %s <file.c>\n", args[0]);
    return 1;
  }

  lex_init(args[1]);

  token_t out = { 0 };
  for (;;) {
    lex_pop(&out);
    if (out.type == TOK_EOF) {
      break;
    }

    int size = out.end - out.start;
    printf("%u %.*s\n", out.type, size, out.start);
  }

  return 0;
}
