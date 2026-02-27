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
  case TOK_CHAR:       return "char";
  case TOK_SHORT:      return "short";
  case TOK_RETURN:     return "return";
  case TOK_WHILE:      return "while";
  case TOK_DO:         return "do";
  case TOK_FOR:        return "for";
  case TOK_IF:         return "if";
  case TOK_BREAK:      return "break";
  case TOK_CONTINUE:   return "continue";
  case TOK_COMMA:      return ",";
  case TOK_ASSIGN:     return "=";
  case TOK_ADD:        return "+";
  case TOK_SUB:        return "-";
  case TOK_MUL:        return "*";
  case TOK_DIV:        return "/";
  case TOK_MOD:        return "%";
  case TOK_BIT_AND:    return "&";
  case TOK_LOG_AND:    return "&&";
  case TOK_BIT_OR:     return "|";
  case TOK_LOG_OR:     return "||";
  case TOK_BIT_XOR:    return "^";
  case TOK_SHL:        return "<<";
  case TOK_SHR:        return ">>";
  case TOK_EQ:         return "==";
  case TOK_NEQ:        return "!=";
  case TOK_LT:         return "<";
  case TOK_LTE:        return "<=";
  case TOK_GT:         return ">";
  case TOK_GTE:        return ">=";
  case TOK_BIT_NOT:    return "~";
  case TOK_LOG_NOT:    return "!";
  case TOK_EOF:        return "end of file";
  default:
    assert(!"unreachable");
    return NULL;
  }
}

bool tok_is_type(token_t *t) {
  switch (t->type) {
  case TOK_VOID:
  case TOK_CHAR:
  case TOK_SHORT:
  case TOK_INT:
    return true;
  default:
    return false;
  }
}

bool tok_is(token_t *t, token_type_t type) {
  return t->type == type;
}

int tok_prec(token_t *t) {
  switch (t->type) {
  case TOK_COMMA:     return 1;
  case TOK_ASSIGN:    return 2;
  case TOK_LOG_OR:    return 3;
  case TOK_LOG_AND:   return 4;
  case TOK_BIT_OR:    return 5;
  case TOK_BIT_XOR:   return 6;
  case TOK_BIT_AND:   return 7;
  case TOK_EQ:
  case TOK_NEQ:       return 8;
  case TOK_LT:
  case TOK_LTE:
  case TOK_GT:
  case TOK_GTE:       return 9;
  case TOK_SHL:
  case TOK_SHR:       return 10;
  case TOK_ADD:
  case TOK_SUB:       return 11;
  case TOK_MUL:
  case TOK_MOD:
  case TOK_DIV:       return 12;
  default:
    return 0;
  }
}

bool tok_is_operator(token_t *t) {
  return tok_prec(t) > 0;
}

void tok_print(token_t *t) {
  const int size = t->end - t->start;
  printf("%.*s", size, t->start);
}
