#include "defs.h"


ast_node_p aNodeNew(ast_node_type_t type) {
  ast_node_p node = malloc(sizeof(ast_node_t));
  assert(node);
  memset(node, 0, sizeof(ast_node_t));
  node->type = type;
  return node;
}

ast_node_p aNodeInsert(ast_node_p chain, ast_node_p toInsert) {

  toInsert->next = NULL;

  if (!chain) {
    toInsert->last = toInsert;
    return toInsert;
  }

  chain->last->next = toInsert;
  chain->last       = toInsert;
  return chain;
}

static void aDumpNode(ast_node_p n, int level) {

  for (int i=0; i<level; ++i) {
    printf(". ");
  }

  switch (n->type) {
  case AST_ROOT:
    printf("AST_ROOT\n");
    break;
  case AST_DECL_VAR:
    printf("AST_DECL_VAR %s %.*s\n",
      tName(&n->declVar.type),
      tSize(&n->declVar.ident),
      n->declVar.ident.start);
    break;
  case AST_DECL_FUNC:
    printf("AST_DECL_FUNC %s %.*s\n",
      tName(&n->declFunc.type),
      tSize(&n->declFunc.ident),
      n->declFunc.ident.start);
    break;
  case AST_STMT_RETURN:
    printf("AST_STMT_RETURN\n");
    break;
  case AST_STMT_EXPR:
    printf("AST_STMT_EXPR\n");
    break;
  case AST_STMT_COMPOUND:
    printf("AST_STMT_COMPOUND\n");
    break;
  case AST_STMT_IF:
    printf("AST_STMT_IF\n");
    break;
  case AST_STMT_WHILE:
    printf("AST_STMT_WHILE\n");
    break;
  case AST_STMT_BREAK:
    printf("AST_STMT_BREAK\n");
    break;
  case AST_STMT_CONTINUE:
    printf("AST_STMT_CONTINUE\n");
    break;
  case AST_EXPR_IDENT:
    printf("AST_EXPR_IDENT %.*s\n",
      tSize(&n->exprIdent.token),
      n->exprIdent.token.start);
    break;
  case AST_EXPR_INT_LIT:
    printf("AST_EXPR_INT_LIT %.*s\n",
      tSize(&n->exprIntLit.token),
      n->exprIntLit.token.start);
    break;
  case AST_EXPR_BIN_OP:
    printf("AST_EXPR_BIN_OP %.*s\n",
      tSize(&n->exprBinOp.op),
      n->exprBinOp.op.start);
    break;
  case AST_STMT_DO:
    printf("AST_STMT_DO\n");
    break;
  case AST_STMT_FOR:
    printf("AST_STMT_FOR\n");
    break;
  case AST_EXPR_UNARY_OP:
    printf("AST_EXPR_UNARY_OP %.*s\n",
      tSize(&n->exprUnaryOp.op),
      n->exprUnaryOp.op.start);
    break;
  case AST_EXPR_CALL:
    printf("AST_EXPR_CALL %.*s\n",
      tSize(&n->exprCall.ident),
      n->exprCall.ident.start);
    break;
  default:
    assert(!"unhandled node type");
  }
}

typedef void (*ast_walk_func_t)(ast_node_p node, int level);

static void aWalk(ast_node_p n, ast_walk_func_t preFunc, int level) {

#define WALK(NODE) { aWalk(NODE, preFunc, level+1); }

  if (!n) {
    return;
  }

  do {
    if (preFunc) {
      preFunc(n, level);
    }

    switch (n->type) {
    case AST_ROOT:
      WALK(n->root.node);
      break;
    case AST_DECL_VAR:
      WALK(n->declVar.expr);
      break;
    case AST_DECL_FUNC:
      WALK(n->declFunc.args);
      WALK(n->declFunc.body);
      break;
    case AST_STMT_RETURN:
      WALK(n->stmtReturn.expr);
      break;
    case AST_STMT_EXPR:
      WALK(n->stmtExpr.expr);
      break;
    case AST_STMT_COMPOUND:
      WALK(n->stmtCompound.stmt);
      break;
    case AST_STMT_IF:
      WALK(n->stmtIf.expr);
      WALK(n->stmtIf.isTrue);
      WALK(n->stmtIf.isFalse);
      break;
    case AST_STMT_WHILE:
      WALK(n->stmtWhile.expr);
      WALK(n->stmtWhile.body);
      break;
    case AST_STMT_BREAK:
      break;
    case AST_STMT_CONTINUE:
      break;
    case AST_EXPR_IDENT:
      break;
    case AST_EXPR_INT_LIT:
      break;
    case AST_EXPR_BIN_OP:
      WALK(n->exprBinOp.lhs);
      WALK(n->exprBinOp.rhs);
      break;
    case AST_STMT_DO:
      WALK(n->stmtDo.body);
      WALK(n->stmtDo.expr);
      break;
    case AST_STMT_FOR:
      WALK(n->stmtFor.init);
      WALK(n->stmtFor.cond);
      WALK(n->stmtFor.update);
      WALK(n->stmtFor.body);
      break;
    case AST_EXPR_UNARY_OP:
      WALK(n->exprUnaryOp.rhs);
      break;
    case AST_EXPR_CALL:
      WALK(n->exprCall.arg);
      break;
    default:
      assert(!"unhandled node type");
    }

  } while (n = n->next);

#undef WALK
}

void aDump(ast_node_p n) {
  aWalk(n, aDumpNode, 0);
}
