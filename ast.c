#include "defs.h"


ast_node_p ast_node_new(ast_node_type_t type) {
  ast_node_p node = malloc(sizeof(ast_node_t));
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
}

void ast_walk(ast_node_p n, int level) {

  if (!n) {
    return;
  }

  do {

    for (uint32_t i=0; i<level; ++i) {
      printf(". ");
    }

    switch (n->type) {
    case AST_ROOT:
      printf("AST_ROOT\n");
      ast_walk(n->root.node, level+1);
      break;
    case AST_DECL_VAR:
      printf("AST_DECL_VAR ");
      tok_print(&n->decl_var.type);
      printf(" ");
      tok_print(&n->decl_var.ident);
      printf("\n");
      ast_walk(n->decl_var.expr, level+1);
      break;
    case AST_DECL_FUNC:
      printf("AST_DECL_FUNC ");
      tok_print(&n->decl_func.type);
      printf(" ");
      tok_print(&n->decl_func.ident);
      printf("\n");
      ast_walk(n->decl_func.args, level+1);
      ast_walk(n->decl_func.body, level+1);
      break;
    case AST_STMT_RETURN:
      printf("AST_STMT_RETURN\n");
      ast_walk(n->stmt_return.expr, level+1);
      break;
    case AST_STMT_EXPR:
      printf("AST_STMT_EXPR\n");
      ast_walk(n->stmt_expr.expr, level+1);
      break;
    case AST_STMT_COMPOUND:
      printf("AST_STMT_COMPOUND\n");
      ast_walk(n->stmt_compound.stmt, level+1);
      break;
    case AST_STMT_IF:
      printf("AST_STMT_IF\n");
      ast_walk(n->stmt_if.expr, level+1);
      ast_walk(n->stmt_if.is_true, level+1);
      ast_walk(n->stmt_if.is_false, level+1);
      break;
    case AST_STMT_WHILE:
      printf("AST_STMT_WHILE\n");
      ast_walk(n->stmt_while.expr, level+1);
      ast_walk(n->stmt_while.body, level+1);
      break;
    case AST_STMT_BREAK:
      printf("AST_STMT_BREAK\n");
      break;
    case AST_STMT_CONTINUE:
      printf("AST_STMT_CONTINUE\n");
      break;
    case AST_EXPR_IDENT:
      printf("AST_EXPR_IDENT ");
      tok_print(&n->expr_ident.token);
      printf("\n");
      break;
    case AST_EXPR_INT_LIT:
      printf("AST_EXPR_INT_LIT ");
      tok_print(&n->expr_int_lit.token);
      printf("\n");
      break;
    case AST_EXPR_BIN_OP:
      printf("AST_EXPR_BIN_OP ");
      tok_print(&n->expr_bin_op.op);
      printf("\n");
      ast_walk(n->expr_bin_op.lhs, level+1);
      ast_walk(n->expr_bin_op.rhs, level+1);
      break;
    case AST_STMT_DO:
      printf("AST_STMT_DO:\n");
      ast_walk(n->stmt_do.body, level+1);
      ast_walk(n->stmt_do.expr, level+1);
      break;
    case AST_STMT_FOR:
      printf("AST_STMT_FOR:\n");
      ast_walk(n->stmt_for.init,   level+1);
      ast_walk(n->stmt_for.cond,   level+1);
      ast_walk(n->stmt_for.update, level+1);
      ast_walk(n->stmt_for.body,   level+1);
      break;
    case AST_EXPR_UNARY_OP:
      printf("AST_EXPR_UNARY_OP ");
      tok_print(&n->expr_unary_op.op);
      printf("\n");
      ast_walk(n->expr_unary_op.rhs, level+1);
      break;
    default:
      assert(!"unhandled node type");
    }

  } while (n = n->next);
}
