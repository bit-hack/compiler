#include "defs.h"


// TODO:
// - check return in void function
// - check continue, break in loop
// - check call argument count
// - check call of void return type in expr
// - type propagation/checking?
// - lvalue rvalue assignment checking
// - check all types makes sense


typedef struct {
  ast_node_p* stack;
  uint32_t    max;
  uint32_t    head;
} ast_stack_t;

struct {
  ast_stack_t stack;
  ast_stack_t hist;
} sema;

static void stackPush(ast_stack_t *stack, ast_node_p node) {
  if (!stack->stack || stack->head >= stack->max) {
    stack->max += 128;
    ast_node_p *alloc = realloc(stack->stack, stack->max * sizeof(ast_node_p));
    assert(alloc);
    stack->stack = alloc;
  }
  stack->stack[stack->head++] = node;
}

static void stackPop(ast_stack_t *stack) {
  assert(stack->head);
  --stack->head;
}

static uint32_t stackSave(ast_stack_t *stack) {
  return stack->head;
}

static void stackRestore(ast_stack_t *stack, uint32_t head) {
  stack->head = head;
}

static void stackClear(ast_stack_t *stack) {
  stack->head = 0;
}

static bool stackEmpty(ast_stack_t *stack) {
  return stack->head == 0;
}


//----------------------------------------------------------------------------
// SemaCheckLoops
//
// Check break and continue are only in loops
//----------------------------------------------------------------------------

static void semaCheckLoops(ast_node_p n) {

  ast_stack_t *stack = &sema.stack;

  uint32_t scope = 0;

  for (; n; n = n->next) {

    switch (n->type) {
    case AST_ROOT:
      semaCheckLoops(n->root.node);
      break;
    case AST_DECL_FUNC:
      semaCheckLoops(n->declFunc.body);
      break;
    case AST_STMT_COMPOUND:
      semaCheckLoops(n->stmtCompound.stmt);
      break;
    case AST_STMT_IF:
      semaCheckLoops(n->stmtIf.isTrue);
      semaCheckLoops(n->stmtIf.isFalse);
      break;
    case AST_STMT_WHILE:
      scope = stackSave(stack);
      stackPush(stack, n);
      semaCheckLoops(n->stmtWhile.body);
      stackRestore(stack, scope);
      break;
    case AST_STMT_DO:
      scope = stackSave(stack);
      stackPush(stack, n);
      semaCheckLoops(n->stmtDo.body);
      stackRestore(stack, scope);
      break;
    case AST_STMT_FOR:
      scope = stackSave(stack);
      stackPush(stack, n);
      semaCheckLoops(n->stmtFor.body);
      stackRestore(stack, scope);
      break;
    case AST_STMT_BREAK:
      if (stackEmpty(stack)) {
        ERROR_LN(n->stmtBreak.token.line, "Break statement outside of loop");
      }
      break;
    case AST_STMT_CONTINUE:
      if (stackEmpty(stack)) {
        ERROR_LN(n->stmtBreak.token.line, "Continue statement outside of loop");
      }
      break;
    }
  }
}

//----------------------------------------------------------------------------
// SemaCheckTypes
//----------------------------------------------------------------------------

static void semaCheckFuncReturnType(ast_node_p t) {

  if (!t) {
    // t is a void type
    return;
  }
  
}

static void semaCheckDeclVarType(ast_node_p t) {

  // plain void is not allowed (void*) is

}

static void semaCheckTypesDecl(ast_node_p n, token_t* t) {

  for (uint32_t i = 0; i < sema.stack.head; ++i) {
    ast_node_p d = sema.stack.stack[i];
    bool error = false;
    switch (d->type) {
    case AST_DECL_FUNC:
      if (!d->declFunc.body) {
        // this is just a declaration not a definition
        break;
      }
      error |= tEqual(&d->declFunc.ident, t);
      break;
    case AST_DECL_VAR:
      error |= tEqual(&d->declVar.ident, t);
      break;
    }
    if (error) {
      ERROR_LN(
        t->line,
        "'%.*s' already declared", (int)(t->end - t->start), t->start);
    }
  }
}

static void semaCheckTypesUse(ast_node_p n, token_t* t) {
  for (uint32_t i = 0; i < sema.stack.head; ++i) {
    ast_node_p d = sema.stack.stack[i];
    switch (d->type) {
    case AST_DECL_FUNC:
      if (tEqual(&d->declFunc.ident, t)) {
        n->decorate.type = d;
        n->decorate.lvalue = false;
        return;
      }
      break;
    case AST_DECL_VAR:
      if (tEqual(&d->declVar.ident, t)) {
        n->decorate.type = d;
        n->decorate.lvalue = true;
        return;
      }
      break;
    }
  }
  ERROR_LN(
    t->line,
    "'%.*s' not declared", (int)(t->end - t->start), t->start);
}

static void semaCheckTypesPropagage(ast_node_p n) {

  switch (n->type) {
  case AST_EXPR_BIN_OP:
    // TODO
  case AST_EXPR_UNARY_OP:
    // TODO
  case AST_EXPR_CALL:
    // TODO
  case AST_EXPR_CAST:
    // TODO
    break;
  default:
    assert(!"unreachable");
  }
}

static void semaCheckReturnType(ast_node_p n) {
  assert(n->type == AST_STMT_RETURN);

  ast_node_p func = sema.hist.stack[0];

  if (n->stmtReturn.expr) {
    // TODO
  }
  else {
    // TODO
  }

}

static void semaCheckInLoop(ast_node_p n) {
  // check we are inside of a loop
}

void semaCheckTypes(ast_node_p n) {

  stackPush(&sema.hist, n);

  ast_stack_t *stack = &sema.stack;
  
  uint32_t scope = 0;

  for (; n; n = n->next) {

    switch (n->type) {
    case AST_ROOT:
      semaCheckTypes(n->root.node);
      break;
    case AST_DECL_VAR:
      semaCheckDeclVarType(n->declVar.type);
      // func args might not have a name...
      if (tIs(&n->declVar.ident, TOK_IDENT)) {
        semaCheckTypesDecl(n, &n->declVar.ident);
        stackPush(stack, n);
      }
      break;
    case AST_DECL_FUNC:
      semaCheckFuncReturnType(n->declFunc.type);      // check return type
      semaCheckTypesDecl(n, &n->declFunc.ident);      // check function name
      stackPush(stack, n);                            // record function name
      {
        scope = stackSave(stack);                     // enter scope
        semaCheckTypes(n->declFunc.args);             // check args
        semaCheckTypes(n->declFunc.body);             // walk body
        stackRestore(stack, scope);                   // leave scope
      }
      break;
    case AST_STMT_RETURN:
      semaCheckTypes(n->stmtReturn.expr);
      semaCheckReturnType(n);
      break;
    case AST_STMT_EXPR:
      semaCheckTypes(n->stmtExpr.expr);
      break;
    case AST_STMT_COMPOUND:
      {
        scope = stackSave(stack);
        semaCheckTypes(n->stmtCompound.stmt);
        stackRestore(stack, scope);
      }
      break;
    case AST_STMT_IF:
      semaCheckTypes(n->stmtIf.expr);
      {
        scope = stackSave(stack);                     // enter scope
        semaCheckTypes(n->stmtIf.isTrue);
        stackRestore(stack, scope);                   // reset scope
        semaCheckTypes(n->stmtIf.isFalse);
        stackRestore(stack, scope);                   // leave scope
      }
      break;
    case AST_STMT_WHILE:
      semaCheckTypes(n->stmtWhile.expr);
      {
        scope = stackSave(stack);                     // enter scope
        semaCheckTypes(n->stmtWhile.body);
        stackRestore(stack, scope);                   // leave scope
      }
      break;
    case AST_STMT_BREAK:
      semaCheckInLoop(n);
      break;
    case AST_STMT_CONTINUE:
      semaCheckInLoop(n);
      break;
    case AST_STMT_DO:
      {
        scope = stackSave(stack);
        semaCheckTypes(n->stmtDo.body);
        stackRestore(stack, scope);
      }
      semaCheckTypes(n->stmtDo.expr);
      break;
    case AST_STMT_FOR:
      semaCheckTypes(n->stmtFor.init);
      semaCheckTypes(n->stmtFor.cond);
      semaCheckTypes(n->stmtFor.update);
      {
        scope = stackSave(stack);
        semaCheckTypes(n->stmtFor.body);
        stackRestore(stack, scope);
      }
      break;
    case AST_EXPR_IDENT:
      semaCheckTypesUse(n, &n->exprIdent.ident);
      break;
    case AST_EXPR_INT_LIT:
      break;
    case AST_EXPR_BIN_OP:
      semaCheckTypes(n->exprBinOp.lhs);
      semaCheckTypes(n->exprBinOp.rhs);
      semaCheckTypesPropagage(n);
      break;
    case AST_EXPR_UNARY_OP:
      semaCheckTypes(n->exprUnaryOp.rhs);
      semaCheckTypesPropagage(n);
      break;
    case AST_EXPR_CALL:
      semaCheckTypesUse(n, &n->exprCall.ident);
      semaCheckTypes(n->exprCall.arg);
      semaCheckTypesPropagage(n);
      break;
    case AST_EXPR_CAST:
      semaCheckTypes(n->exprCast.expr);
      semaCheckTypesPropagage(n);
      break;
    default:
      assert(!"unreachable");
    }
  }

  stackPop(&sema.hist);
}

void sCheck(ast_node_p n) {
  semaCheckTypes(n);

  stackClear(&sema.stack);
  semaCheckLoops(n);
}
