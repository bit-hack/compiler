#include "defs.h"


static ast_node_p parse_expr(int minPrec);
static ast_node_p parse_stmt(void);

static parser_t parser;

static void exprStackPush(ast_node_p n) {
  parser.exprStack[ parser.exprStackHead++ ] = n;
}

static ast_node_p exprStackPop(void) {
  return parser.exprStack[ --parser.exprStackHead ];
}

static ast_node_p parse_expr_primary(void) {
 
  token_t prim;
  lex_pop(&prim);

  // parenthesized expression
  if (tok_is(&prim, TOK_LPAREN)) {
    ast_node_p n = parse_expr(/*minPrec=*/0);
    lex_expect(TOK_RPAREN);
    return n;
  }

  if (tok_is(&prim, TOK_IDENT)) {
    ast_node_p n = ast_node_new(AST_EXPR_IDENT);
    n->expr_ident.token = prim;
    return n;
  }

  if (tok_is(&prim, TOK_INT_LIT)) {
    ast_node_p n = ast_node_new(AST_EXPR_INT_LIT);
    n->expr_int_lit.token = prim;
    return n;
  }

  ERROR("Primary expression expected");
  return NULL;
}

static bool parse_check_precedence(token_t *t, int minPrec) {
  if (tok_is(t, TOK_ASSIGN)) {
    return tok_precedence(t) < minPrec;
  }
  else {
    return tok_precedence(t) <= minPrec;
  }
}

static ast_node_p parse_expr(int minPrec) {

  // lhs
  exprStackPush(parse_expr_primary());

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
    ast_node_p rhs = parse_expr(tok_precedence(&op));
    ast_node_p lhs = exprStackPop();    

    ast_node_p bin_op = ast_node_new(AST_EXPR_BIN_OP);
    bin_op->expr_bin_op.op  = op;
    bin_op->expr_bin_op.lhs = lhs;
    bin_op->expr_bin_op.rhs = rhs;
    exprStackPush(bin_op);
  }

  return exprStackPop();
}

static ast_node_p parse_stmt_if(void) {

  ast_node_p n = ast_node_new(AST_STMT_IF);

  // condition
  lex_expect(TOK_LPAREN);
  n->stmt_if.expr = parse_expr(0);
  lex_expect(TOK_RPAREN);

  // is true branch
  n->stmt_if.is_true = parse_stmt();

  // if false branch
  if (lex_found(TOK_ELSE, NULL)) {
    n->stmt_if.is_false = parse_stmt();
  }

  return n;
}

static ast_node_p parse_stmt_while(void) {

  ast_node_p n = ast_node_new(AST_STMT_WHILE);

  // condition
  lex_expect(TOK_LPAREN);
  n->stmt_while.expr = parse_expr(0);
  lex_expect(TOK_RPAREN);
  
  n->stmt_while.body = parse_stmt();

  return n;
}

static ast_node_p parse_stmt_return(void) {

  ast_node_p n = ast_node_new(AST_STMT_RETURN);

  if (lex_found(TOK_SEMICOLON, NULL)) {
    return n;
  }

  ast_node_p e = parse_expr(/*minPrec=*/0);
  n->stmt_return.expr = ast_node_insert(n->stmt_return.expr, e);

  lex_expect(TOK_SEMICOLON);
  return n;
}

static ast_node_p parse_stmt_local_decl(void) {

  ast_node_p n = ast_node_new(AST_DECL_VAR);

  lex_pop(&n->decl_var.type);
  lex_pop(&n->decl_var.ident);

  if (lex_found(TOK_ASSIGN, NULL)) {
    n->decl_var.expr = ast_node_insert(n->decl_var.expr, parse_expr(/*minPrec=*/0));
  }

  lex_expect(TOK_SEMICOLON);
  return n;
}

static ast_node_p parse_stmt_compound(void) {

  ast_node_p c = ast_node_new(AST_STMT_COMPOUND);

  while (!lex_found(TOK_RBRACE, NULL)) {
    ast_node_p n = parse_stmt();
    c->stmt_compound.stmt = ast_node_insert(c->stmt_compound.stmt, n);
  }

  return c;
}

static ast_node_p parse_stmt(void) {

  token_t la;
  lex_peek(&la);

  // local decl
  if (tok_is_type(&la)) {
    return parse_stmt_local_decl();
  }

  // if conditional
  if (tok_is(&la, TOK_IF)) {
    lex_pop(&la);
    return parse_stmt_if();
  }

  // while loop
  if (tok_is(&la, TOK_WHILE)) {
    lex_pop(&la);
    return parse_stmt_while();
  }

  // empty statement
  if (tok_is(&la, TOK_SEMICOLON)) {
    lex_pop(&la);
    return ast_node_new(AST_STMT_COMPOUND);
  }

  // compound statements
  if (tok_is(&la, TOK_LBRACE)) {
    lex_pop(&la);
    return parse_stmt_compound();
  }

  // return statement
  if (tok_is(&la, TOK_RETURN)) {
    lex_pop(&la);
    return parse_stmt_return();
  }

  // fallback to trying to parse an expression
  ast_node_p expr = parse_expr(/*minPrec=*/0);
  ast_node_p stmt_expr = ast_node_new(AST_STMT_EXPR);
  stmt_expr->stmt_expr.expr = ast_node_insert(stmt_expr->stmt_expr.expr, stmt_expr);

  lex_expect(TOK_SEMICOLON);

  return expr;
}

static void parse_func(ast_node_t *decl) {

  // parse arguments
  if (!lex_found(TOK_RPAREN, NULL)) {
    do {

      token_t type;
      lex_pop(&type);
      if (tok_is(&type, TOK_VOID)) {
        break;
      }

      token_t ident;
      lex_pop(&ident);

      ast_node_p arg = ast_node_new(AST_DECL_VAR);
      arg->decl_var.type = type;
      arg->decl_var.ident = ident;
      decl->decl_func.args = ast_node_insert(decl->decl_func.args, arg);

    } while (lex_found(TOK_COMMA, NULL));
  }
  lex_expect(TOK_RPAREN);

  if (lex_found(TOK_LBRACE, NULL)) {
    // parse function body
    while (!lex_found(TOK_RBRACE, NULL)) {
        ast_node_p stmt = parse_stmt();
        decl->decl_func.body = ast_node_insert(decl->decl_func.body, stmt);
    }
  }
  else {
    // function decl
    lex_expect(TOK_SEMICOLON);
  }
}

void parse(void) {

  ast_node_p r = ast_node_new(AST_ROOT);
  parser.astRoot = r;

  token_t token;
  while (!lex_found(TOK_EOF, &token)) {

    token_t type;
    lex_pop(&type);

    token_t ident;
    lex_pop(&ident);

    // function declaration
    if (lex_found(TOK_LPAREN, NULL)) {

      ast_node_p f = ast_node_new(AST_DECL_FUNC);
      r->root.node = ast_node_insert(r->root.node, f);
      f->decl_func.type = type;
      f->decl_func.ident = ident;

      parse_func(f);
      continue;
    }

    ast_node_p v = ast_node_new(AST_DECL_VAR);
    r->root.node = ast_node_insert(r->root.node, v);
    v->decl_var.type = type;
    v->decl_var.ident = ident;

    // global decl with initializer
    if (lex_found(TOK_ASSIGN, NULL)) {
      v->decl_var.expr = parse_expr(/*minPrec*/0);
    }
    
    lex_expect(TOK_SEMICOLON);
  }

  ast_walk(parser.astRoot, 0);
}
