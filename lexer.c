#include "defs.h"


static lex_t lex;

uint32_t lLineNum(void) {
  return lex.lineNum;
}

bool lInit(const char *file) {

  FILE *fd = fopen(file, "rb");
  if (!fd) {
    ERROR("Unable to open '%s'\n", file);
    return false;
  }

  // file size
  fseek(fd, 0, SEEK_END);
  const long fdSize = ftell(fd);
  fseek(fd, 0, SEEK_SET);
  if (fdSize <= 0) {
    return false;
  }

  // read file data
  char *src = malloc(fdSize + 1);
  if (!src) {
    return false;
  }
  fread(src, 1, fdSize, fd);
  src[fdSize] = '\0';

  // close file handle
  fclose(fd);

  // init lexer
  lex.start = src;
  lex.end = src + fdSize;
  lex.lineNum = 1;
  lex.lineStart = src;
  lex.ptr = src;

  return true;
}

static void lSkipWhitespace(void) {

  bool inComment = false;

  const char *p = lex.ptr;
  for (;*p; ++p) {

    // track newlines
    if (*p == '\n') {
      lex.lineNum++;
      lex.lineStart = (p + 1);
    }

    // multi line comments
    if (inComment) {
      if (p[0] == '*' && p[1] == '/') {
        inComment = false;
        ++p;
      }
      continue;
    }
    if (p[0] == '/' && p[1] == '*') {
      inComment = true;
      continue;
    }

    // skip single line comments
    if (p[0] == '/' && p[1] == '/') {
      for (; *p != '\0' && *p != '\n'; ++p);
      continue;
    }

    // skip whitespace
    switch (*p) {
    case '\n':
    case ' ':
    case '\t':
    case '\r':
      continue;
    }

    // reached a non skipable char
    break;
  }

  lex.ptr = p;
}

static bool lIsAlpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool lIsNumeric(char c) {
  return (c >= '0' && c <= '9');
}

static bool lMatch(const char *s) {
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

static bool lIdent(token_t *out) {
  const char *p = lex.ptr;
  if (!lIsAlpha(*p) && *p != '_') {
    return false;
  }
  for (++p;;++p) {
    if (lIsAlpha(*p)) {
      continue;
    }
    if (lIsNumeric(*p)) {
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

static bool lIntLit(token_t *out) {
  const char *p = lex.ptr;
  for (; lIsNumeric(*p); ++p);
  if (lex.ptr == p) {
    return false;
  }
  out->end = p;
  lex.ptr = p;
  return true;
}

void lPop(token_t *out) {

  lSkipWhitespace();

  // prepare outgoing token
  out->type  = TOK_UNKNOWN;
  out->line  = lex.lineNum;
  out->start = lex.ptr;
  out->end   = NULL;

#define TEST(FOR, TOK) if (lMatch(FOR)) { out->type = TOK; break; }

  // first stage simple classifier
  switch (*lex.ptr) {
  case '\0': out->type = TOK_EOF;       break;
  case ';':  out->type = TOK_SEMICOLON; break;
  case '(':  out->type = TOK_LPAREN;    break;
  case ')':  out->type = TOK_RPAREN;    break;
  case '{':  out->type = TOK_LBRACE;    break;
  case '}':  out->type = TOK_RBRACE;    break;
  case '+':  out->type = TOK_ADD;       break;
  case '-':  out->type = TOK_SUB;       break;
  case '*':  out->type = TOK_MUL;       break;
  case '/':  out->type = TOK_DIV;       break;
  case '%':  out->type = TOK_MOD;       break;
  case ',':  out->type = TOK_COMMA;     break;
  case '^':  out->type = TOK_BIT_XOR;   break;
  case '~':  out->type = TOK_BIT_NOT;   break;
  case '=':
    TEST("==", TOK_EQ);
    out->type = TOK_ASSIGN;
    break;
  case '<':
    TEST("<=", TOK_LTE);
    TEST("<<", TOK_SHL);
    out->type = TOK_LT;
    break;
  case '>':
    TEST(">>", TOK_SHR);
    TEST(">=", TOK_GTE);
    out->type = TOK_GT;
    break;
  case '&':
    TEST("&&", TOK_LOG_AND);
    out->type = TOK_BIT_AND;
    break;
  case '|':
    TEST("||", TOK_LOG_OR);
    out->type = TOK_BIT_OR;
    break;
  case '!':
    TEST("!=", TOK_NEQ);
    out->type = TOK_LOG_NOT;
    break;
  case 'b':   TEST("break",     TOK_BREAK);     break;
  case 'c':   TEST("char",      TOK_CHAR);
              TEST("continue",  TOK_CONTINUE);  break;
  case 'd':   TEST("do",        TOK_DO);        break;
  case 'e':   TEST("else",      TOK_ELSE);      break;
  case 'f':   TEST("for",       TOK_FOR);       break;
  case 'i':   TEST("int",       TOK_INT);
              TEST("if",        TOK_IF);        break;
  case 'r':   TEST("return",    TOK_RETURN);    break;
  case 's':   TEST("short",     TOK_SHORT);     break;
  case 'v':   TEST("void",      TOK_VOID);      break;
  case 'w':   TEST("while",     TOK_WHILE);     break;
  }

  // second stage classifier
  if (out->type == TOK_UNKNOWN) {
    do {
      if (lIdent(out))  { out->type = TOK_IDENT;   break; }
      if (lIntLit(out)) { out->type = TOK_INT_LIT; break; }

      ERROR("unknown token on line %u", lex.lineNum);

    } while (0);
  }

  // fixup for one character tokens
  if (lex.ptr == out->start) {
    ++lex.ptr;
  }

  // fill in token end
  out->end = lex.ptr;
}

void lPeek(token_t *out) {
  const lex_t save = lex;
  lPop(out);
  lex = save;
}

void lExpect(token_type_t type) {
  token_t t;
  lPop(&t);
  if (t.type == type) {
    return;
  }
  ERROR("expected %s token", tTypeName(type));
}

bool lFound(token_type_t type, token_t *out) {

  token_t temp;
  out = out ? out : &temp;

  lPeek(out);
  if (out->type == type) {
    lPop(out);
    return true;
  }
  return false;
}
