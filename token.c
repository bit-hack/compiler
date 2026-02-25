#include "defs.h"


const char *tok_name(token_type_t type) {
  switch (type) {
  case TOK_SEMICOLON:  return ";";
  case TOK_LPAREN:     return "(";
  case TOK_RPAREN:     return ")";
  case TOK_LBRACE:     return "{";
  case TOK_RBRACE:     return "}";
  case TOK_IDENT:      return "identifier";
  case TOK_INT:        return "int";
  case TOK_INT_LIT:    return "integer literal";
  case TOK_VOID:       return "void";
  case TOK_RETURN:     return "return";
  case TOK_COMMA:      return ",";
  case TOK_ASSIGN:     return "=";
  case TOK_ADD:        return "+";
  case TOK_SUB:        return "-";
  case TOK_MUL:        return "*";
  case TOK_EOF:        return "end of file";
  }
}

bool tok_is_type(token_t *t) {
  switch (t->type) {
  case TOK_VOID:
  case TOK_INT:
    return true;
  default:
    return false;
  }
}

bool tok_is(token_t *t, token_type_t type) {
  return t->type == type;
}

bool tok_is_operator(token_t *t) {
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

int tok_precedence(token_t *t) {
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
