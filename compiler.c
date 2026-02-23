#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

enum {
  TOK_LPAREN,
  TOK_RPAREN,
  TOK_IDENT,
  TOK_INT,
  TOK_VOID,
  TOK_RETURN,
  TOK_EOF,
};

typedef struct {
  const char *start;
  const char *end;
  uint32_t type;
  uint32_t line;
} token_t;

typedef struct {
  const char *start;
  const char *end;
  const char *ptr;
  const char *line_start;
  uint32_t line_num;
} lex_t;

lex_t lex;

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

static void lex_peek(token_t *out) {
  lex_skip_whitespace();
  out->line = lex.line_num;
  out->start = lex.ptr;
  
  const char *p = lex.ptr;
  switch (*p) {
  case '\0': out->type = TOK_EOF;     break;
  case '(':  out->type = TOK_LPAREN;  break;
  case ')':  out->type = TOK_RPAREN;  break;
  case 'v':
    if (lex_match("void")) { out->type = TOK_VOID; break; }
    break;
  case 'i':
    if (lex_match("int")) { out->type = TOK_INT; break; }
    break;
  default:
    // try parse identifier
    // try parse integer literal
    break;
  }

  out->end = lex.ptr;
  lex.ptr = p;
}

static void lex_pop(token_t *out) {
  lex_peek(out);
  lex.ptr = out->end;
}

static void lex_expect(uint32_t type) {
  token_t t;
  lex_pop(&t);
  if (t.type != type) {
    // ERROR
  }
}

static bool lex_found(uint32_t type, token_t *out) {
  lex_peek(out);
  if (out->type == type) {
    lex.ptr  = out->end;
    return true;
  }
  return false;
}

int main(int argc, char **args) {

  if (argc <= 1) {
    printf("usage: %s <file.c>\n", args[0]);
    return 1;
  }

  lex_init(args[1]);

  return 0;
}