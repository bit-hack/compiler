#include "defs.h"


// TODO:
// - check return in void function
// - check continue, break in loop
// - check call argument count
// - check call of void return type in expr
// - type propagation/checking?
// - lvalue rvalue assignment checking
// - check all types makes sense


struct {
  ast_node_p* stack;
  uint32_t    stackMax;
  uint32_t    stackHead;
} sema;


static void stackPush(ast_node_p node) {
  if (!sema.stack || sema.stackHead >= sema.stackMax) {
    sema.stackMax += 128;
    ast_node_p *alloc = realloc(sema.stack, sema.stackMax * sizeof(ast_node_p));
    assert(alloc);
    sema.stack = alloc;
  }
  sema.stack[sema.stackHead++] = node;
}

static uint32_t stackSave(void) {
  return sema.stackHead;
}

static void stackRestore(uint32_t head) {
  sema.stackHead = head;
}

static void stackClear(void) {
  sema.stackHead = 0;
}

static bool stackEmpty(void) {
  return sema.stackHead == 0;
}


//----------------------------------------------------------------------------
// SemaCheckLoops
//
// Check break and continue are only in loops
//----------------------------------------------------------------------------

static void semaCheckLoops(ast_node_p n) {

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
      scope = stackSave();
      stackPush(n);
      semaCheckLoops(n->stmtWhile.body);
      stackRestore(scope);
      break;
    case AST_STMT_DO:
      scope = stackSave();
      stackPush(n);
      semaCheckLoops(n->stmtDo.body);
      stackRestore(scope);
      break;
    case AST_STMT_FOR:
      scope = stackSave();
      stackPush(n);
      semaCheckLoops(n->stmtFor.body);
      stackRestore(scope);
      break;
    case AST_STMT_BREAK:
      if (stackEmpty()) {
        ERROR_LN(n->stmtBreak.token.line, "Break statement outside of loop");
      }
      break;
    case AST_STMT_CONTINUE:
      if (stackEmpty()) {
        ERROR_LN(n->stmtBreak.token.line, "Continue statement outside of loop");
      }
      break;
    }
  }
}

//----------------------------------------------------------------------------
// SemaCheckDecls
//
// Check all identifiers have a declaration
//----------------------------------------------------------------------------

static void semaCheckDeclsDecl(token_t* t) {
  for (uint32_t i = 0; i < sema.stackHead; ++i) {
    ast_node_p n = sema.stack[i];
    bool error = false;
    switch (n->type) {
    case AST_DECL_FUNC:
      if (!n->declFunc.body) {
        // this is just a declaration not a definition
        break;
      }
      error |= tEqual(&n->declFunc.ident, t);
      break;
    case AST_DECL_VAR:
      error |= tEqual(&n->declVar.ident, t);
      break;
    }
    if (error) {
      ERROR_LN(
        t->line,
        "'%.*s' already declared", (int)(t->end - t->start), t->start);
    }
  }
}

static void semaCheckDeclsUse(token_t* t) {
  for (uint32_t i = 0; i < sema.stackHead; ++i) {
    ast_node_p n = sema.stack[i];
    switch (n->type) {
    case AST_DECL_FUNC:
      if (tEqual(&n->declFunc.ident, t)) {
        return;
      }
      break;
    case AST_DECL_VAR:
      if (tEqual(&n->declVar.ident, t)) {
        return;
      }
      break;
    }
  }
  ERROR_LN(
    t->line,
    "'%.*s' not declared", (int)(t->end - t->start), t->start);
}

void semaCheckDecls(ast_node_p n) {

  uint32_t scope = 0;

  for (; n; n = n->next) {

    switch (n->type) {
    case AST_ROOT:
      semaCheckDecls(n->root.node);
      break;
    case AST_DECL_VAR:
      semaCheckDeclsDecl(&n->declVar.ident);
      stackPush(n);
      break;
    case AST_DECL_FUNC:
      {
        semaCheckDeclsDecl(&n->declFunc.ident);
        stackPush(n);
        scope = stackSave();
        semaCheckDecls(n->declFunc.args);
        semaCheckDecls(n->declFunc.body);
        stackRestore(scope);
      }
      break;
    case AST_STMT_RETURN:
      semaCheckDecls(n->stmtReturn.expr);
      break;
    case AST_STMT_EXPR:
      semaCheckDecls(n->stmtExpr.expr);
      break;
    case AST_STMT_COMPOUND:
      {
        scope = stackSave();
        semaCheckDecls(n->stmtCompound.stmt);
        stackRestore(scope);
      }
      break;
    case AST_STMT_IF:
      semaCheckDecls(n->stmtIf.expr);
      {
        scope = stackSave();
        semaCheckDecls(n->stmtIf.isTrue);
        stackRestore(scope);
        semaCheckDecls(n->stmtIf.isFalse);
        stackRestore(scope);
      }
      break;
    case AST_STMT_WHILE:
      semaCheckDecls(n->stmtWhile.expr);
      {
        scope = stackSave();
        semaCheckDecls(n->stmtWhile.body);
        stackRestore(scope);
      }
      break;
    case AST_STMT_BREAK:
      break;
    case AST_STMT_CONTINUE:
      break;
    case AST_STMT_DO:
      {
        scope = stackSave();
        semaCheckDecls(n->stmtDo.body);
        stackRestore(scope);
      }
      semaCheckDecls(n->stmtDo.expr);
      break;
    case AST_STMT_FOR:
      semaCheckDecls(n->stmtFor.init);
      semaCheckDecls(n->stmtFor.cond);
      semaCheckDecls(n->stmtFor.update);
      {
        scope = stackSave();
        semaCheckDecls(n->stmtFor.body);
        stackRestore(scope);
      }
      break;
    case AST_EXPR_IDENT:
      semaCheckDeclsUse(&n->exprIdent.token);
      break;
    case AST_EXPR_INT_LIT:
      break;
    case AST_EXPR_BIN_OP:
      semaCheckDecls(n->exprBinOp.lhs);
      semaCheckDecls(n->exprBinOp.rhs);
      break;
    case AST_EXPR_UNARY_OP:
      semaCheckDecls(n->exprUnaryOp.rhs);
      break;
    case AST_EXPR_CALL:
      semaCheckDeclsUse(&n->exprCall.ident);
      semaCheckDecls(n->exprCall.arg);
      break;
    case AST_EXPR_CAST:
      semaCheckDecls(n->exprCast.expr);
      break;
    default:
      assert(!"unreachable");
    }
  }
}

void sCheck(ast_node_p n) {
  semaCheckDecls(n);

  stackClear();
  semaCheckLoops(n);
}
