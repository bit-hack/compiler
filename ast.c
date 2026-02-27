#include "defs.h"


ast_node_p ast_node_new(ast_node_type_t type) {
  ast_node_p node = malloc(sizeof(ast_node_t));
  assert(node);
  memset(node, 0, sizeof(ast_node_t));
  node->type = type;
  return node;
}

ast_node_p ast_node_insert(ast_node_p chain, ast_node_p to_insert) {
  
  to_insert->next = NULL;

  if (!chain) {
    to_insert->last = to_insert;
    return to_insert;
  }

  chain->last->next = to_insert;
  chain->last       = to_insert;
  return chain;
}

static void ast_dump_node(ast_node_p n, int level) {

  for (int i=0; i<level; ++i) {
    printf(". ");
  }

  switch (n->type) {
  case AST_ROOT:
    printf("AST_ROOT\n");
    break;
  case AST_DECL_VAR:
    printf("AST_DECL_VAR %s %s\n", tok_name(&n->decl_var.type), tok_name(&n->decl_var.ident));
    break;
  case AST_DECL_FUNC:
    printf("AST_DECL_FUNC %s %s\n", tok_name(&n->decl_func.type), tok_name(&n->decl_func.ident));
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
    printf("AST_EXPR_IDENT %s\n", tok_name(&n->expr_ident.token));
    break;
  case AST_EXPR_INT_LIT:
    printf("AST_EXPR_INT_LIT %s\n", tok_name(&n->expr_int_lit.token));
    break;
  case AST_EXPR_BIN_OP:
    printf("AST_EXPR_BIN_OP %s\n", tok_name(&n->expr_bin_op.op));
    break;
  case AST_STMT_DO:
    printf("AST_STMT_DO\n");
    break;
  case AST_STMT_FOR:
    printf("AST_STMT_FOR\n");
    break;
  case AST_EXPR_UNARY_OP:
    printf("AST_EXPR_UNARY_OP %s\n", tok_name(&n->expr_unary_op.op));
    break;
  case AST_EXPR_CALL:
    printf("AST_EXPR_CALL %s\n", tok_name(&n->expr_call.ident));
    break;
  default:
    assert(!"unhandled node type");
  }
}

typedef void (*ast_walk_func_t)(ast_node_p node, int level);

static void ast_walk(ast_node_p n, ast_walk_func_t preFunc, int level) {

#define WALK(NODE) { ast_walk(NODE, preFunc, level+1); }

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
      WALK(n->decl_var.expr);
      break;
    case AST_DECL_FUNC:
      WALK(n->decl_func.args);
      WALK(n->decl_func.body);
      break;
    case AST_STMT_RETURN:
      WALK(n->stmt_return.expr);
      break;
    case AST_STMT_EXPR:
      WALK(n->stmt_expr.expr);
      break;
    case AST_STMT_COMPOUND:
      WALK(n->stmt_compound.stmt);
      break;
    case AST_STMT_IF:
      WALK(n->stmt_if.expr);
      WALK(n->stmt_if.is_true);
      WALK(n->stmt_if.is_false);
      break;
    case AST_STMT_WHILE:
      WALK(n->stmt_while.expr);
      WALK(n->stmt_while.body);
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
      WALK(n->expr_bin_op.lhs);
      WALK(n->expr_bin_op.rhs);
      break;
    case AST_STMT_DO:
      WALK(n->stmt_do.body);
      WALK(n->stmt_do.expr);
      break;
    case AST_STMT_FOR:
      WALK(n->stmt_for.init);
      WALK(n->stmt_for.cond);
      WALK(n->stmt_for.update);
      WALK(n->stmt_for.body);
      break;
    case AST_EXPR_UNARY_OP:
      WALK(n->expr_unary_op.rhs);
      break;
    case AST_EXPR_CALL:
      WALK(n->expr_call.arg);
      break;
    default:
      assert(!"unhandled node type");
    }

  } while (n = n->next);

#undef WALK
}

void ast_dump(ast_node_p n) {
  ast_walk(n, ast_dump_node, 0);
}
