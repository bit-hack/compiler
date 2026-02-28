#include <assert.h>

#include "defs.h"


#define AST_NODE_INSERT( INTO, NODE ) { \
  INTO = aNodeInsert(INTO, NODE); \
}

static ast_node_p pExpr(int minPrec);
static ast_node_p pStmt(void);

static parser_t parser;

static void exprStackPush(ast_node_p n) {
  static const size_t exprStackSize = sizeof(parser.exprStack) / sizeof(ast_node_p);
  assert(parser.exprStackHead < exprStackSize);
  parser.exprStack[ parser.exprStackHead++ ] = n;
}

static ast_node_p exprStackPop(void) {
  assert(parser.exprStackHead > 0);
  return parser.exprStack[ --parser.exprStackHead ];
}

static ast_node_p pExprCall(token_t *ident) {

  ast_node_p n = aNodeNew(AST_EXPR_CALL);
  n->exprCall.ident = *ident;

  if (!lFound(TOK_RPAREN, NULL)) {
    do {

      ast_node_p e = pExpr(/*minPrec=*/0);
      AST_NODE_INSERT(n->exprCall.arg, e);

    } while (lFound(TOK_COMMA, NULL));
  }
  lExpect(TOK_RPAREN);

  return n;
}

static ast_node_p pExprPrimary(void) {

  token_t prim;
  lPop(&prim);

  // unary operator
  if (tIs(&prim, TOK_LOG_NOT) ||
      tIs(&prim, TOK_BIT_NOT) ||
      tIs(&prim, TOK_SUB)     ||
      tIs(&prim, TOK_BIT_AND) ||
      tIs(&prim, TOK_MUL)) {
    ast_node_p n = aNodeNew(AST_EXPR_UNARY_OP);
    n->exprUnaryOp.op = prim;
    n->exprUnaryOp.rhs = pExprPrimary();
    return n;
  }

  // parenthesized expression
  if (tIs(&prim, TOK_LPAREN)) {
    ast_node_p n = pExpr(/*minPrec=*/0);
    lExpect(TOK_RPAREN);
    return n;
  }

  if (tIs(&prim, TOK_IDENT)) {

    if (lFound(TOK_LPAREN, NULL)) {
      return pExprCall(&prim);
    }

    ast_node_p n = aNodeNew(AST_EXPR_IDENT);
    n->exprIdent.token = prim;
    return n;
  }

  if (tIs(&prim, TOK_INT_LIT)) {
    ast_node_p n = aNodeNew(AST_EXPR_INT_LIT);
    n->exprIntLit.token = prim;
    return n;
  }

  ERROR("Primary expression expected");
  return NULL;
}

static bool pCheckPrec(token_t *t, int minPrec) {
  if (tIs(t, TOK_ASSIGN)) {
    return tPrec(t) < minPrec;
  }
  else {
    return tPrec(t) <= minPrec;
  }
}

static ast_node_p pExpr(int minPrec) {

  // lhs
  exprStackPush(pExprPrimary());

  for (;;) {

    // look for an operator
    token_t op;
    lPeek(&op);
    if (!tIsOperator(&op)) {
      break;
    }

    // if our found operator binds less tightly than a prior one stop.
    if (pCheckPrec(&op, minPrec)) {
      break;
    }

    // pop the operator
    lPop(&op);

    // rhs
    ast_node_p rhs = pExpr(tPrec(&op));
    ast_node_p lhs = exprStackPop();    

    ast_node_p bin_op = aNodeNew(AST_EXPR_BIN_OP);
    bin_op->exprBinOp.op  = op;
    bin_op->exprBinOp.lhs = lhs;
    bin_op->exprBinOp.rhs = rhs;
    exprStackPush(bin_op);
  }

  return exprStackPop();
}

static ast_node_p pStmtIf(void) {

  ast_node_p n = aNodeNew(AST_STMT_IF);

  // condition
  lExpect(TOK_LPAREN);
  n->stmtIf.expr = pExpr(/*minPrec=*/0);
  lExpect(TOK_RPAREN);

  // is true branch
  n->stmtIf.isTrue = pStmt();

  // if false branch
  if (lFound(TOK_ELSE, NULL)) {
    n->stmtIf.isFalse = pStmt();
  }

  return n;
}

static ast_node_p pStmtWhile(void) {

  ast_node_p n = aNodeNew(AST_STMT_WHILE);

  // condition
  lExpect(TOK_LPAREN);
  n->stmtWhile.expr = pExpr(/*minPrec=*/0);
  lExpect(TOK_RPAREN);
  
  // statement
  n->stmtWhile.body = pStmt();

  return n;
}

static ast_node_p pStmtReturn(void) {

  ast_node_p n = aNodeNew(AST_STMT_RETURN);

  if (lFound(TOK_SEMICOLON, NULL)) {
    return n;
  }

  ast_node_p e = pExpr(/*minPrec=*/0);
  AST_NODE_INSERT(n->stmtReturn.expr, e);

  lExpect(TOK_SEMICOLON);
  return n;
}

static ast_node_p pStmtBreak(void) {

  lExpect(TOK_SEMICOLON);
  return aNodeNew(AST_STMT_BREAK);
}

static ast_node_p pStmtContinue(void) {

  lExpect(TOK_SEMICOLON);
  return aNodeNew(AST_STMT_CONTINUE);
}

static ast_node_p pStmtLocalDecl(void) {

  ast_node_p n = aNodeNew(AST_DECL_VAR);

  lPop(&n->declVar.type);
  lPop(&n->declVar.ident);

  if (lFound(TOK_ASSIGN, NULL)) {
    AST_NODE_INSERT(n->declVar.expr, pExpr(/*minPrec=*/0));
  }

  lExpect(TOK_SEMICOLON);
  return n;
}

static ast_node_p pStmtCompound(void) {

  ast_node_p c = aNodeNew(AST_STMT_COMPOUND);

  while (!lFound(TOK_RBRACE, NULL)) {
    ast_node_p n = pStmt();
    AST_NODE_INSERT(c->stmtCompound.stmt, n);
  }

  return c;
}

static ast_node_p pStmtDo(void) {

  ast_node_p n = aNodeNew(AST_STMT_DO);

  AST_NODE_INSERT(n->stmtDo.body, pStmt());

  lExpect(TOK_WHILE);
  lExpect(TOK_LPAREN);
  AST_NODE_INSERT(n->stmtDo.expr, pExpr(/*minPrec=*/0));
  lExpect(TOK_RPAREN);
  lExpect(TOK_SEMICOLON);

  return n;
}

static ast_node_p pStmtFor(void) {

  ast_node_p n = aNodeNew(AST_STMT_FOR);

  lExpect(TOK_LPAREN);
  AST_NODE_INSERT(n->stmtFor.init, pExpr(/*minPrec=*/0));
  lExpect(TOK_SEMICOLON);
  AST_NODE_INSERT(n->stmtFor.cond, pExpr(/*minPrec=*/0));
  lExpect(TOK_SEMICOLON);
  AST_NODE_INSERT(n->stmtFor.update, pExpr(/*minPrec=*/0));
  lExpect(TOK_RPAREN);

  AST_NODE_INSERT(n->stmtFor.body, pStmt());

  return n;
}

static ast_node_p pStmt(void) {

  token_t la;
  lPeek(&la);

  // local decl
  if (tIsType(&la)) {
    return pStmtLocalDecl();
  }

  // if conditional
  if (tIs(&la, TOK_IF)) {
    lPop(&la);
    return pStmtIf();
  }

  // while loop
  if (tIs(&la, TOK_WHILE)) {
    lPop(&la);
    return pStmtWhile();
  }

  if (tIs(&la, TOK_DO)) {
    lPop(&la);
    return pStmtDo();
  }

  if (tIs(&la, TOK_FOR)) {
    lPop(&la);
    return pStmtFor();
  }

  if (tIs(&la, TOK_BREAK)) {
    lPop(&la);
    return pStmtBreak();
  }

  if (tIs(&la, TOK_CONTINUE)) {
    lPop(&la);
    return pStmtContinue();
  }

  // empty statement
  if (tIs(&la, TOK_SEMICOLON)) {
    lPop(&la);
    return aNodeNew(AST_STMT_COMPOUND);
  }

  // compound statements
  if (tIs(&la, TOK_LBRACE)) {
    lPop(&la);
    return pStmtCompound();
  }

  // return statement
  if (tIs(&la, TOK_RETURN)) {
    lPop(&la);
    return pStmtReturn();
  }

  // fallback to trying to parse an expression
  ast_node_p expr = pExpr(/*minPrec=*/0);
  ast_node_p stmtExpr = aNodeNew(AST_STMT_EXPR);
  AST_NODE_INSERT(stmtExpr->stmtExpr.expr, stmtExpr);

  lExpect(TOK_SEMICOLON);

  return expr;
}

static void pFunc(ast_node_t *decl) {

  // parse arguments
  if (!lFound(TOK_RPAREN, NULL)) {
    do {

      token_t type;
      lPop(&type);
      if (tIs(&type, TOK_VOID)) {
        break;
      }

      token_t ident;
      lPop(&ident);

      ast_node_p arg = aNodeNew(AST_DECL_VAR);
      AST_NODE_INSERT(decl->declFunc.args, arg);
      arg->declVar.type = type;
      arg->declVar.ident = ident;

    } while (lFound(TOK_COMMA, NULL));
  }
  lExpect(TOK_RPAREN);

  if (lFound(TOK_LBRACE, NULL)) {
    // parse function body
    while (!lFound(TOK_RBRACE, NULL)) {
        ast_node_p stmt = pStmt();
        AST_NODE_INSERT(decl->declFunc.body, stmt);
    }
  }
  else {
    // function decl
    lExpect(TOK_SEMICOLON);
  }
}

ast_node_p pParse(void) {

  ast_node_p r = aNodeNew(AST_ROOT);
  parser.astRoot = r;

  token_t token;
  while (!lFound(TOK_EOF, &token)) {

    token_t type;
    lPop(&type);

    token_t ident;
    lPop(&ident);

    // function declaration
    if (lFound(TOK_LPAREN, NULL)) {

      ast_node_p f = aNodeNew(AST_DECL_FUNC);
      AST_NODE_INSERT(r->root.node, f);
      f->declFunc.type = type;
      f->declFunc.ident = ident;

      pFunc(f);
      continue;
    }

    ast_node_p v = aNodeNew(AST_DECL_VAR);
    AST_NODE_INSERT(r->root.node, v);
    v->declVar.type = type;
    v->declVar.ident = ident;

    // global decl with initializer
    if (lFound(TOK_ASSIGN, NULL)) {
      v->declVar.expr = pExpr(/*minPrec*/0);
    }
    
    lExpect(TOK_SEMICOLON);
  }

  return r;
}
