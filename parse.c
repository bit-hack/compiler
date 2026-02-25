#include "defs.h"


static void parse_expr(int minPrec);

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

void parse(void) {

  token_t token;
  while (!lex_found(TOK_EOF, &token)) {

    token_t type;
    lex_pop(&type);

    token_t ident;
    lex_pop(&type);

    if (lex_found(TOK_LPAREN, NULL)) {
      // function declaration
      parse_func(&type, &ident);
      continue;
    }

    if (lex_found(TOK_ASSIGN, NULL)) {
      parse_expr(/*minPrec*/0);
    }

    lex_expect(TOK_SEMICOLON);
  }
}
